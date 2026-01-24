#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: ./calibrage <video_path>" << endl;
        return -1;
    }

    string videoPath = argv[1];
    Size boardSize(9, 6); 
    float squareSize = 25.0f;

    vector<vector<Point3f>> objectPoints;
    vector<vector<Point2f>> imagePoints;
    vector<Point3f> obj;
    for (int i = 0; i < boardSize.height; i++)
        for (int j = 0; j < boardSize.width; j++)
            obj.push_back(Point3f(j * squareSize, i * squareSize, 0.0f));

    Mat frame, gray;
    vector<Point2f> lastCorners;

    // On ouvre la vidéo
    VideoCapture cap(videoPath);
    
    cout << "Analyse automatique... Objectif : 25 captures minimum." << endl;

    while (true) {
        if (!cap.read(frame)) {
            // Si la vidéo finit, on propose de calculer ou de recommencer
            if (imagePoints.size() >= 15) break; 
            else {
                cout << "Fin de video, mais seulement " << imagePoints.size() << " captures. Relancement..." << endl;
                cap.set(CAP_PROP_POS_FRAMES, 0); // On recommence la vidéo au début
                continue;
            }
        }

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        vector<Point2f> corners;

        // Détection plus permissive pour attraper plus de frames
        bool found = findChessboardCorners(gray, boardSize, corners,
            CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FILTER_QUADS);

        if (found) {
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));

            double dist = 100.0;
            if (!imagePoints.empty()) dist = norm(corners[0] - lastCorners[0]);

            // Seuil réduit à 20 pixels pour capturer plus souvent
            if (dist > 20.0) { 
                imagePoints.push_back(corners);
                objectPoints.push_back(obj);
                lastCorners = corners;
                cout << "Capture n°" << imagePoints.size() << " OK" << endl;
            }
            drawChessboardCorners(frame, boardSize, corners, found);
        }

        imshow("Calibration en cours", frame);
        if (waitKey(1) == 27 || imagePoints.size() >= 40) break;
    }

    // --- CALCUL DE HAUTE PRÉCISION ---
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
    Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
    vector<Mat> rvecs, tvecs;

    cout << "Calcul final avec " << imagePoints.size() << " points..." << endl;
    double rms = calibrateCamera(objectPoints, imagePoints, gray.size(), 
                                 cameraMatrix, distCoeffs, rvecs, tvecs,
                                 CALIB_FIX_K3 | CALIB_FIX_K4 | CALIB_FIX_K5);

    cout << "Erreur RMS : " << rms << " (Cible : < 0.5)" << endl;

    fs::path p(videoPath);
    string yamlPath = p.replace_extension(".yaml").string();
    FileStorage fs_out(yamlPath, FileStorage::WRITE);
    fs_out << "camera_matrix" << cameraMatrix;
    fs_out << "distortion_coefficients" << distCoeffs;
    fs_out.release();

    cout << "Fichier pret : " << yamlPath << endl;
    return 0;
}