#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <GL/glut.h>

const int SIZE = 600;

struct Point
{
    double x;
    double y;
};

struct Edge
{
    Point start;
    Point end;
};

struct Polygon
{
    std::vector<Edge> edges;
};

// Helper function to check if a point is inside a polygon
bool isInsidePolygon(const Point &point, const Polygon &polygon)
{
    bool inside = false;
    int numEdges = polygon.edges.size();

    for (int i = 0, j = numEdges - 1; i < numEdges; j = i++)
    {
        const Point &start = polygon.edges[i].start;
        const Point &end = polygon.edges[i].end;

        if (((start.y > point.y) != (end.y > point.y)) &&
            (point.x < (end.x - start.x) * (point.y - start.y) / (end.y - start.y) + start.x))
        {
            inside = !inside;
        }
    }

    return inside;
}

// Helper function to find the intersection point of two line segments
Point getIntersection(const Point &p1, const Point &p2, const Point &p3, const Point &p4)
{
    double x1 = p1.x, y1 = p1.y;
    double x2 = p2.x, y2 = p2.y;
    double x3 = p3.x, y3 = p3.y;
    double x4 = p4.x, y4 = p4.y;

    double denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denominator == 0)
    {
        // Lines are parallel, return a point at infinity
        return {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    }

    double intersectX = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denominator;
    double intersectY = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denominator;

    return {intersectX, intersectY};
}

// Weiler-Atherton algorithm for polygon clipping
Polygon weilerAthertonClip(const Polygon &subjectPolygon, const Polygon &clipPolygon)
{
    Polygon clippedPolygon;

    int numSubjectEdges = subjectPolygon.edges.size();
    int numClipEdges = clipPolygon.edges.size();

    for (int i = 0; i < numSubjectEdges; ++i)
    {
        const Edge &subjectEdge = subjectPolygon.edges[i];
        const Edge &nextSubjectEdge = subjectPolygon.edges[(i + 1) % numSubjectEdges];

        std::vector<Point> intersectPoints;

        for (int j = 0; j < numClipEdges; ++j)
        {
            const Edge &clipEdge = clipPolygon.edges[j];
            const Edge &nextClipEdge = clipPolygon.edges[(j + 1) % numClipEdges];

            Point intersect = getIntersection(subjectEdge.start, subjectEdge.end, clipEdge.start, clipEdge.end);
            if (intersect.x != std::numeric_limits<double>::infinity() && intersect.y != std::numeric_limits<double>::infinity())
            {
                intersectPoints.push_back(intersect);
            }
        }

        if (!intersectPoints.empty())
        {
            // Sort the intersection points based on their distance from the starting point of the subject edge
            std::sort(intersectPoints.begin(), intersectPoints.end(), [&](const Point &p1, const Point &p2)
                      {
                double dist1 = std::sqrt(std::pow((p1.x - subjectEdge.start.x), 2) + std::pow((p1.y - subjectEdge.start.y), 2));
                double dist2 = std::sqrt(std::pow((p2.x - subjectEdge.start.x), 2) + std::pow((p2.y - subjectEdge.start.y), 2));
                return dist1 < dist2; });

            if (isInsidePolygon(subjectEdge.start, clipPolygon))
            {
                clippedPolygon.edges.push_back(subjectEdge);
            }

            for (const Point &intersect : intersectPoints)
            {
                clippedPolygon.edges.push_back({intersect, intersect});
            }
        }
    }

    return clippedPolygon;
}

// Function to draw polygons using OpenGL
void drawPolygon(const Polygon &polygon)
{
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
    for (const Edge &edge : polygon.edges)
    {
        glVertex2i(edge.start.x, edge.start.y);
        glVertex2i(edge.end.x, edge.end.y);
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_POINT_SMOOTH);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Define the subject polygon

    // plgn1.pts = {{553, 495}, {181, 175}, {486, 71}, {61, 86}, {90, 200}};
    // plgn2.pts = {{390, 424}, {579, 585}, {207, 12}, {68, 245}};
    Polygon subjectPolygon;
    subjectPolygon.edges.push_back({{0, 1}, {0, 4}});
    subjectPolygon.edges.push_back({{0, 4}, {7, 7}});
    subjectPolygon.edges.push_back({{6, 6}, {0, 1}});
    // subjectPolygon.edges.push_back({{2, 2}, {0, 0}});

    // Define the clip polygon
    Polygon clipPolygon;
    clipPolygon.edges.push_back({{0, 0}, {2, 6}});
    clipPolygon.edges.push_back({{2, 6}, {6, 6}});
    clipPolygon.edges.push_back({{6, 6}, {6, 2}});
    clipPolygon.edges.push_back({{6, 2}, {0, 0}});

    // Perform polygon clipping using the Weiler-Atherton algorithm
    Polygon clippedPolygon = weilerAthertonClip(subjectPolygon, clipPolygon);

    // Draw the subject polygon
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for subject polygon
    drawPolygon(subjectPolygon);

    // Draw the clip polygon
    glColor3f(0.0f, 1.0f, 0.0f); // Green color for clip polygon
    drawPolygon(clipPolygon);

    // Draw the clipped polygon
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for clipped polygon
    drawPolygon(clippedPolygon);

    glFlush();
}

void init()
{
    int SIZE = 10;
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-3.0, SIZE - 1, 0.0, SIZE - 1);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(SIZE, SIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Weiler-Atherton Clipping Algorithm");

    glutDisplayFunc(display);

    init();
    glutMainLoop();

    return 0;
}