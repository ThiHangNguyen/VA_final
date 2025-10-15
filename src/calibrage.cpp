// Camera_DamierOuA4_Preview_Yaml_Safe_patched.cpp
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

enum class Mode { UNKNOWN=0, CHESS, A4 };

// ---------- Damier (inchangé côté algo) ----------
static bool detectChess(const cv::Mat& gray, cv::Size patternSize,
std::vector<cv::Point2f>& corners)
{
    int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;
    if (!cv::findChessboardCorners(gray, patternSize, corners, flags)) return false;
    cv::cornerSubPix(gray, corners, {11,11}, {-1,-1},
    {cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 30, 1e-3});
    return true;
}

// ---------- A4 (sans damier) ----------
static void orderQuad(std::vector<cv::Point2f>& q){ // TL,TR,BR,BL
    std::sort(q.begin(), q.end(), [](auto&a,auto&b){ return (a.y<b.y)||(a.y==b.y && a.x<b.x);});
    cv::Point2f tl=q[0], tr=q[1], bl=q[2], br=q[3];
    if (cv::norm(tr-tl) < cv::norm(bl-tl)) std::swap(tr, bl);
    q = {tl,tr,br,bl};
    }
    static bool detectA4(const cv::Mat& bgr, std::vector<cv::Point2f>& quad){
    cv::Mat gray, edges;
    if (bgr.channels()==3) cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY); else gray=bgr;
    cv::GaussianBlur(gray, gray, {5,5}, 0);
    cv::Canny(gray, edges, 60, 180);
    cv::dilate(edges, edges, cv::getStructuringElement(cv::MORPH_RECT,{3,3}));

    std::vector<std::vector<cv::Point>> cs;
    cv::findContours(edges, cs, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double bestArea=0; std::vector<cv::Point> best;
    double minArea = 0.08 * (double)bgr.total();
    for (auto& c: cs){
    double per = cv::arcLength(c,true);
    std::vector<cv::Point> p; cv::approxPolyDP(c, p, 0.02*per, true);
    if (p.size()!=4 || !cv::isContourConvex(p)) continue;
    double area = std::abs(cv::contourArea(p));
    if (area < minArea) continue;
    if (area > bestArea){ bestArea=area; best=p; }
    }
    if (best.empty()) return false;

    quad.resize(4);
    for (int i=0;i<4;++i) quad[i] = cv::Point2f((float)best[i].x,(float)best[i].y);
    orderQuad(quad);
    cv::cornerSubPix(gray, quad, {7,7}, {-1,-1},
    {cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 40, 1e-3});
    return true;
}

// ---------- HUD ----------
static void drawHUD(cv::Mat& img, const std::string& l1, const std::string& l2, int kept, int maxV){
    int th=18, y=28;
    cv::rectangle(img, {10,10}, {img.cols-10, 10+th*3+10}, cv::Scalar(0,180,0), 2);
    cv::putText(img, l1, {20,y}, cv::FONT_HERSHEY_SIMPLEX, 0.62, cv::Scalar(255,255,255), 2); y+=th;
    cv::putText(img, l2, {20,y}, cv::FONT_HERSHEY_SIMPLEX, 0.62, cv::Scalar(255,255,255), 2); y+=th;
    char buf[128]; std::snprintf(buf,sizeof(buf),"Vues retenues: %d/%d (p: pause, s: save, q: quit)",kept,maxV);
    cv::putText(img, buf, {20,y}, cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(230,230,230), 1);
    }

    // ---------- Helpers YAML "propre" ----------
    static std::string fmt_num(double v){
    if (!std::isfinite(v)) return "0";
    const double r = std::round(v);
    if (std::abs(v - r) < 1e-12) {
    std::ostringstream oss; oss << (long long)r;
    return oss.str();
    }
    std::ostringstream oss;
    // defaultfloat (ni fixed ni scientific), haute précision lisible
    oss.setf(std::ios::fmtflags(0), std::ios::floatfield);
    oss << std::setprecision(15) << v;
    return oss.str();
    }

    static void write_opencv_matrix(std::ofstream& f, const char* name, const cv::Mat& M){
    CV_Assert(M.isContinuous());
    f << name << ": !!opencv-matrix\n";
    f << " rows: " << M.rows << "\n";
    f << " cols: " << M.cols << "\n";
    f << " dt: d\n";
    f << " data: [";
    for (int r = 0; r < M.rows; ++r){
    for (int c = 0; c < M.cols; ++c){
    double v;
    switch (M.depth()){
    case CV_64F: v = M.at<double>(r,c); break;
    case CV_32F: v = M.at<float>(r,c); break;
    case CV_32S: v = (double)M.at<int>(r,c); break;
    case CV_16U: v = (double)M.at<uint16_t>(r,c); break;
    case CV_8U: v = (double)M.at<uint8_t>(r,c); break;
    default: v = M.at<double>(r,c); break;
    }
    f << fmt_num(v);
    if (r != M.rows-1 || c != M.cols-1) f << ", ";
    }
    }
    f << "]\n";
    }

    static bool write_yaml_like_terminal(const std::string& path,
    const cv::Mat& K, const cv::Mat& dist)
    {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f << "%YAML:1.0\n";
    write_opencv_matrix(f, "camera_matrix", K);
    write_opencv_matrix(f, "distortion_coefficients", dist);
    return true;
}

// ---------- MAIN ----------
int main(int argc, char** argv){
    if (argc < 2) {
    std::cerr << "Usage: " << (argc?argv[0]:"Camera")
    << " <video.mp4> [step=5] [f_ref=885] [force_nominal=0]\n";
    return 1;
    }
    std::string videoPath = argv[1];
    int step = (argc>=3)? std::stoi(argv[2]) : 5;
    double fref = (argc>=4)? std::stod(argv[3]) : 885.0; // focale nominale pour 1280x720
    int forceNominal = (argc>=5)? std::stoi(argv[4]) : 0; // 1 => ignore la focale estimée

    // Damier 
    cv::Size patternSize(6, 9);
    float squareSize = 8.5f;
    const int maxViews = 25, minViews = 8;

    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()){ std::cerr << "ERR: cannot open " << videoPath << "\n"; return -1; }

    cv::Size imageSize(0,0);
    cv::Mat frame, gray, display;

    std::vector<std::vector<cv::Point3f>> objpoints;
    std::vector<std::vector<cv::Point2f>> imgpoints;

    int kept=0, idx=0, saved=0;
    Mode mode = Mode::UNKNOWN;

    cv::namedWindow("Preview", cv::WINDOW_NORMAL);
    cv::resizeWindow("Preview", 960, 540);
    bool paused=false;

    while (true){
    if(!paused){
    if(!cap.read(frame)) break;
    ++idx;
    if (frame.empty()) break;
    if (imageSize.width==0) imageSize = frame.size();
    }
    display = frame.clone();
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    bool used=false, ok=false;

    if (!paused && (idx % step == 0)) {
    if (mode==Mode::UNKNOWN || mode==Mode::CHESS){
    std::vector<cv::Point2f> corners;
    ok = detectChess(gray, patternSize, corners);
    if (ok){
    mode = Mode::CHESS; // === partie damier identique à ton code ===
    std::vector<cv::Point3f> objp; objp.reserve(patternSize.area());
    for(int j=0;j<patternSize.height;++j)
    for(int i=0;i<patternSize.width;++i)
    objp.emplace_back(i*squareSize, j*squareSize, 0.0f);
    imgpoints.push_back(corners);
    objpoints.push_back(objp);
    ++kept; used=true;
    cv::drawChessboardCorners(display, patternSize, corners, true);
    }
    }
    if (!ok && (mode==Mode::UNKNOWN || mode==Mode::A4)){
    std::vector<cv::Point2f> quad;
    if (detectA4(frame, quad)){
    mode = Mode::A4;
    imgpoints.push_back(quad);
    objpoints.push_back({{0,0,0},{297,0,0},{297,210,0},{0,210,0}});
    ++kept; used=true;
    for(int i=0;i<4;++i) cv::line(display, quad[i], quad[(i+1)%4], cv::Scalar(0,255,0), 2);
    }
    }
    }

    std::string l1 = (mode==Mode::CHESS? "Damier 6x9" : (mode==Mode::A4? "A4" : "Recherche motif..."));
    std::string l2 = used ? "DETECTION OK" : "(pas de detection cette frame)";
    drawHUD(display, l1, l2, kept, maxViews);

    cv::imshow("Preview", display);
    int k=cv::waitKey(1);
    if(k=='q'||k==27) break;
    if(k=='p') paused=!paused;
    if(k=='s'){ std::filesystem::path p(videoPath);
    std::string out=(p.parent_path()/(p.stem().string()+"_snap_"+std::to_string(saved++)+".png")).string();
    cv::imwrite(out, display); std::cout<<"snapshot: "<<out<<"\n"; }

    if(kept>=maxViews) break;
    }
    cv::destroyWindow("Preview");
    cap.release();

    if (mode==Mode::UNKNOWN){
    std::cerr << "ERR: aucun motif détecté (ni damier ni A4)\n";
    return -2;
    }

    // -------- YAML: centre au milieu, distorsion nulle --------
    cv::Mat K_yaml = cv::Mat::eye(3,3,CV_64F);
    K_yaml.at<double>(0,2) = imageSize.width * 0.5;
    K_yaml.at<double>(1,2) = imageSize.height * 0.5;
    cv::Mat dist_yaml = cv::Mat::zeros(1,5,CV_64F);

    // focale nominale mise à l'échelle
    const double fx_nom = fref * ((double)imageSize.width / 1280.0);
    const double fy_nom = fref * ((double)imageSize.height / 720.0);

    auto pick_focal = [](double f_est, double f_nom)->double{
    if (!std::isfinite(f_est)) return f_nom;
    // garde l'estimation seulement si elle est proche de la nominale (±60%)
    if (f_est < 0.4*f_nom || f_est > 1.6*f_nom) return f_nom;
    return f_est;
    };

    if (mode==Mode::CHESS && kept >= 8 && !forceNominal){
    // == EXACTEMENT le bloc de calibration damier ==
    cv::Mat K, dist; std::vector<cv::Mat> rvecs, tvecs;
    int flags = cv::CALIB_FIX_K3 | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5 | cv::CALIB_FIX_K6;
    double rms = cv::calibrateCamera(objpoints, imgpoints, imageSize, K, dist, rvecs, tvecs, flags);
    std::cout << "RMS Error = " << rms << "\n";
    std::cout << "Image size = " << imageSize.width << " x " << imageSize.height << "\n";
    std::cout << "K =\n" << K << "\n";
    std::cout << "Dist (full) = " << dist.t() << "\n";

    // sécurisation pour le YAML : si K "délire", on retombe sur la focale nominale
    K_yaml.at<double>(0,0) = pick_focal(K.at<double>(0,0), fx_nom);
    K_yaml.at<double>(1,1) = pick_focal(K.at<double>(1,1), fy_nom);
    } else {
    // A4 ou forçage → focale nominale uniquement
    K_yaml.at<double>(0,0) = fx_nom;
    K_yaml.at<double>(1,1) = fy_nom;
    if (mode==Mode::A4) std::cout << "A4: focale nominale (fx="<<fx_nom<<", fy="<<fy_nom<<")\n";
    if (forceNominal) std::cout << "Force nominal: focale imposee (fx="<<fx_nom<<", fy="<<fy_nom<<")\n";
    }

    // -------- Écriture YAML (même dossier que la vidéo) --------
    std::filesystem::path in(videoPath);
    std::filesystem::path out = in.parent_path() / (in.stem().string() + ".yaml");

    // Écriture "propre" sans --- et sans notation scientifique
    if (!write_yaml_like_terminal(out.string(), K_yaml, dist_yaml)) {
    std::cerr << "ERR: cannot open " << out << " for write\n";
    return -4;
    }

    std::cout << "YAML écrit : " << out << "\n";
    std::cout << "K_yaml =\n" << K_yaml << "\n";
    return 0;
}

