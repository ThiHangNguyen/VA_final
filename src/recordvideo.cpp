// record_cam_gui.cpp
// Webcam + enregistrement vidéo avec BOUTONS CLIQUABLES (Start/Stop REC, Snapshot).
// Enregistre automatiquement dans ../data/ (ou ./data/ en fallback).
// Ubuntu / OpenCV 4, backend V4L2. Touche q/ESC pour quitter.

#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <atomic>
#include <filesystem>

namespace fs = std::filesystem;

// ---------- util ----------
static std::string timestamped(const std::string& base, const std::string& ext) {
    using clock = std::chrono::system_clock;
    auto t = clock::to_time_t(clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << base << "_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ext;
    return oss.str();
}

// Choisit ../data si présent (typiquement quand on lance depuis build/), sinon ./data.
// Crée le dossier cible si nécessaire.
static fs::path pickDataDir() {
    fs::path p1 = fs::path("..") / "data";  // racine projet quand tu es dans build/
    fs::path p2 = fs::path("data");         // fallback: data/ local
    if (fs::exists(p1) && fs::is_directory(p1)) return p1;
    if (!fs::exists(p2)) fs::create_directories(p2);
    return p2;
}

static bool tryOpenCam(int cam, int W, int H, int FPS, cv::VideoCapture& cap) {
    cap.release();
    if (!cap.open(cam, cv::CAP_V4L2)) return false;

    // tente MJPG puis YUYV
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, H);
    cap.set(cv::CAP_PROP_FPS,          FPS);

    cv::Mat f;
    if (cap.read(f) && !f.empty()) { return true; }

    cap.release();
    if (!cap.open(cam, cv::CAP_V4L2)) return false;
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, H);
    cap.set(cv::CAP_PROP_FPS,          FPS);

    if (cap.read(f) && !f.empty()) { return true; }

    cap.release();
    return false;
}

// ---------- UI boutons ----------
struct Button {
    cv::Rect rect;
    std::string label;
    bool hot = false;
};

struct UIState {
    Button btnRec{ {10,10,140,40}, "Start REC" };
    Button btnSnap{ {160,10,140,40}, "Snapshot" };
    std::atomic<bool> wantToggleRec{false};
    std::atomic<bool> wantSnapshot{false};
} ui;

// callback souris
static void onMouse(int event, int x, int y, int, void*) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        if (ui.btnRec.rect.contains({x,y}))  ui.wantToggleRec = true;
        if (ui.btnSnap.rect.contains({x,y})) ui.wantSnapshot = true;
    }
}

// dessin boutons
static void drawButton(cv::Mat& img, const Button& b, bool active=false) {
    cv::Scalar fill = active ? cv::Scalar(30,180,30) : cv::Scalar(40,40,40);
    cv::Scalar border = active ? cv::Scalar(80,230,80) : cv::Scalar(200,200,200);
    cv::rectangle(img, b.rect, fill, cv::FILLED, cv::LINE_AA);
    cv::rectangle(img, b.rect, border, 2, cv::LINE_AA);
    int baseline=0;
    double scale=0.6;
    int thickness=2;
    cv::Size ts = cv::getTextSize(b.label, cv::FONT_HERSHEY_SIMPLEX, scale, thickness, &baseline);
    cv::Point org(b.rect.x + (b.rect.width - ts.width)/2, b.rect.y + (b.rect.height + ts.height)/2 - 4);
    cv::putText(img, b.label, org, cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(255,255,255), thickness, cv::LINE_AA);
}

static void drawRECdot(cv::Mat& img, bool rec) {
    if (!rec) return;
    cv::circle(img, {330, 30}, 10, cv::Scalar(0,0,255), -1, cv::LINE_AA);
    cv::putText(img, "REC", {350, 38}, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,255), 2, cv::LINE_AA);
}

int main(int argc, char** argv) {
    int camIndex = 0;
    int reqW = 1280, reqH = 720, reqFPS = 30;
    if (argc >= 2) try { camIndex = std::stoi(argv[1]); } catch(...) {}
    if (argc >= 4) { try { reqW = std::stoi(argv[2]); reqH = std::stoi(argv[3]); } catch(...) {} }
    if (argc >= 5) try { reqFPS = std::stoi(argv[4]); } catch(...) {}

    cv::VideoCapture cap;
    if (!tryOpenCam(camIndex, reqW, reqH, reqFPS, cap)) {
        std::cerr << "[ERR] Cannot open /dev/video" << camIndex << ". Try index 1 or check group 'video'.\n";
        return 1;
    }

    // vraie config
    int W = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int H = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double FPS = cap.get(cv::CAP_PROP_FPS);
    if (FPS <= 1.0) FPS = reqFPS;

    std::cout << "[INFO] " << W << "x" << H << " @ " << FPS << "fps\n";

    const std::string win = "Webcam Recorder";
    cv::namedWindow(win, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(win, onMouse, nullptr);

    // writer lazy (créé au clic Start REC)
    cv::VideoWriter writer;
    bool isRecording = false;
    int fourcc_mp4v = cv::VideoWriter::fourcc('m','p','4','v'); // MP4 (MPEG-4 Part 2)
    int fourcc_mjpg = cv::VideoWriter::fourcc('M','J','P','G'); // AVI fallback

    while (true) {
        cv::Mat frame;
        if (!cap.read(frame) || frame.empty()) {
            std::cerr << "[WARN] Empty frame. Stopping.\n";
            break;
        }

        // --- UI input handling ---
        if (ui.wantToggleRec.exchange(false)) {
            if (!isRecording) {
                writer.release();
                fs::path outdir = pickDataDir();
                std::string out = (outdir / timestamped("capture", ".mp4")).string();
                if (!writer.open(out, fourcc_mp4v, FPS, cv::Size(W,H))) {
                    std::cerr << "[WARN] MP4 unavailable, fallback to AVI\n";
                    out = (outdir / timestamped("capture", ".avi")).string();
                    if (!writer.open(out, fourcc_mjpg, FPS, cv::Size(W,H))) {
                        std::cerr << "[ERR] Cannot open any video writer.\n";
                    } else {
                        std::cout << "[OK] Recording: " << out << "\n";
                        isRecording = true;
                        ui.btnRec.label = "Stop REC";
                    }
                } else {
                    std::cout << "[OK] Recording: " << out << "\n";
                    isRecording = true;
                    ui.btnRec.label = "Stop REC";
                }
            } else {
                isRecording = false;
                ui.btnRec.label = "Start REC";
                writer.release();
                std::cout << "[OK] Recording stopped.\n";
            }
        }

        if (ui.wantSnapshot.exchange(false)) {
            fs::path outdir = pickDataDir();
            std::string name = (outdir / timestamped("snapshot", ".png")).string();
            if (cv::imwrite(name, frame)) {
                std::cout << "[OK] Snapshot: " << name << "\n";
            } else {
                std::cout << "[ERR] Snapshot failed.\n";
            }
        }

        // --- write si enregistrement ---
        if (isRecording && writer.isOpened()) {
            writer.write(frame);
        }

        // --- Dessin UI ---
        cv::Mat display = frame.clone();
        drawButton(display, ui.btnRec, isRecording);
        drawButton(display, ui.btnSnap, false);
        drawRECdot(display, isRecording);
        cv::putText(display, "q/ESC: Quit | R: toggle REC | S: snapshot",
                    {10, H - 10}, cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(255,255,255), 2, cv::LINE_AA);

        cv::imshow(win, display);

        int k = cv::waitKey(1) & 0xFF;
        if (k == 27 || k == 'q') break;      // ESC / q
        if (k == 'r' || k == 'R') ui.wantToggleRec = true; // raccourci clavier
        if (k == 's' || k == 'S') ui.wantSnapshot = true;  // raccourci clavier
    }

    cap.release();
    if (writer.isOpened()) writer.release();
    cv::destroyAllWindows();
    return 0;
}
