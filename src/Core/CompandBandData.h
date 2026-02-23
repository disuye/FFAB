#pragma once

#include <QList>
#include <algorithm>

struct CompandPoint {
    double inputDb = 0.0;
    double outputDb = 0.0;
};

struct CompandBandData {
    QList<CompandPoint> points;
    double softKnee = 0.01;
    static constexpr int MAX_POINTS = 20;

    void sortPoints() {
        std::sort(points.begin(), points.end(),
                  [](const CompandPoint& a, const CompandPoint& b) {
                      return a.inputDb < b.inputDb;
                  });
    }

    void addPoint(double inputDb, double outputDb) {
        if (points.size() >= MAX_POINTS) return;
        points.append({inputDb, outputDb});
        sortPoints();
    }

    void removePoint(int index) {
        if (index < 0 || index >= points.size()) return;
        if (points.size() <= 2) return;
        points.removeAt(index);
    }

    int updatePoint(int index, double inputDb, double outputDb) {
        if (index < 0 || index >= points.size()) return index;

        points[index].inputDb = inputDb;
        points[index].outputDb = outputDb;

        CompandPoint edited = points[index];
        sortPoints();

        // Find where the edited point ended up after sorting
        for (int i = 0; i < points.size(); ++i) {
            if (qFuzzyCompare(points[i].inputDb, edited.inputDb) &&
                qFuzzyCompare(points[i].outputDb, edited.outputDb)) {
                return i;
            }
        }
        return index;
    }
};
