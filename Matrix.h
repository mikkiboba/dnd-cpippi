#pragma once
#include "Preprocess.h"
#include <vector>

struct Entity{
	cv::Vec2i screenPosition;
	cv::Vec2i gridPosition;
	cv::Vec3b color;
};

class Matrix {
    private:
        cv::Mat original;
        Preprocess preprocess;
        std::vector<Entity> entitiesVec;
        int countRows;
        int countCols;

        std::set<std::pair<int, int>> occupiedGridCells;


    public:
        Matrix() {
            // original = frame;
            // preprocess.emptyGridMask(original);
        }
        ~Matrix();

        void initPreprocess(cv::Mat& originalm, cv::Mat& background) ;
        void preprocessEntities(cv::Mat& original, cv::Mat& background) ;
        int getDim(bool dirX) ;

        void findElementsInLine(cv::Mat original) ;
        void printEntities();
        inline std::vector<Entity> getEntities() const {return entitiesVec;}
        inline void insertEntity(Entity& e) {entitiesVec.push_back(e);}
        int detectColor(const cv::Vec3b& color);

        inline Preprocess getPreprocess() { return preprocess; }
};