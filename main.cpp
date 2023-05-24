#include <iostream>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#define SIZE 600

using namespace std;

typedef float Color[3];
struct Point
{
    int x, y;

    bool operator==(const Point &other)
    {
        return this->x == other.x && this->y == other.y;
    }
};
typedef struct IntersectionPoint
{
    Point p;
    int point_flag;
    int index0, index1;
    bool in_flag;
    int dis;
} IP;

class Polygon
{
public:
    vector<Point> pts;
    void draw_line(Color c);
};

void Polygon::draw_line(Color c)
{
    glColor3fv(c);
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
    int size = pts.size();
    for (int i = 0; i < size; i++)
        glVertex2i(pts[i].x, pts[i].y);
    glEnd();
}

bool is_point_inside_polygon(Point p, Polygon &plgn)
{
    int cnt = 0, size = plgn.pts.size();
    for (int i = 0; i < size; i++)
    {
        Point p1 = plgn.pts[i];
        Point p2 = plgn.pts[(i + 1) % size];
        if (p1.y == p2.y)
            continue;
        if (p.y < min(p1.y, p2.y))
            continue;
        if (p.y >= max(p1.y, p2.y))
            continue;
        double x = (double)(p.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
        if (x > p.x)
            cnt++;
    }
    return (cnt % 2 == 1);
}

int cross(Point &p0, Point &p1, Point &p2)
{
    return ((p2.x - p0.x) * (p1.y - p0.y) - (p1.x - p0.x) * (p2.y - p0.y));
}

bool is_on_segment(Point &p0, Point &p1, Point &p2)
{
    int minx = min(p0.x, p1.x), maxx = max(p0.x, p1.x);
    int miny = min(p0.y, p1.y), maxy = max(p0.y, p1.y);
    if (p2.x >= minx && p2.x <= maxx && p2.y >= miny && p2.y <= maxy)
        return true;
    return false;
}

bool segments_intersect(Point &p1, Point &p2, Point &p3, Point &p4)
{
    int d1 = cross(p3, p4, p1);
    int d2 = cross(p3, p4, p2);
    int d3 = cross(p1, p2, p3);
    int d4 = cross(p1, p2, p4);
    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
        return true;
    if (d1 == 0 && is_on_segment(p3, p4, p1))
        return true;
    if (d2 == 0 && is_on_segment(p3, p4, p2))
        return true;
    if (d3 == 0 && is_on_segment(p1, p2, p3))
        return true;
    if (d4 == 0 && is_on_segment(p1, p2, p4))
        return true;
    return false;
}

Point get_intersect_point(Point p1, Point p2, Point p3, Point p4)
{
    Point p;
    int b1 = (p2.y - p1.y) * p1.x + (p1.x - p2.x) * p1.y;
    int b2 = (p4.y - p3.y) * p3.x + (p3.x - p4.x) * p3.y;
    int D = (p2.x - p1.x) * (p4.y - p3.y) - (p4.x - p3.x) * (p2.y - p1.y);
    int D1 = b2 * (p2.x - p1.x) - b1 * (p4.x - p3.x);
    int D2 = b2 * (p2.y - p1.y) - b1 * (p4.y - p3.y);
    p.x = D1 / D;
    p.y = D2 / D;
    return p;
}

void generateIntersectPoints(Polygon &plgn1, Polygon &plgn2, list<IP> &iplist)
{
    int clipSize = plgn1.pts.size(), pySize = plgn2.pts.size();

    for (int i = 0; i < clipSize; i++)
    {
        Point p1 = plgn1.pts[i];
        Point p2 = plgn1.pts[(i + 1) % clipSize];
        for (int j = 0; j < pySize; j++)
        {
            Point p3 = plgn2.pts[j];
            Point p4 = plgn2.pts[(j + 1) % pySize];
            if (segments_intersect(p1, p2, p3, p4))
            {
                IP ip;
                ip.index0 = j;
                ip.index1 = i;
                ip.p = get_intersect_point(p1, p2, p3, p4);
                iplist.push_back(ip);
            }
        }
    }
}

int get_distance(Point &p1, Point &p2)
{
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

bool distance_comparator(IP &ip1, IP &ip2)
{
    return ip1.dis < ip2.dis;
}

void generate_list(Polygon &py, list<IP> &iplist, list<IP> &comlist, int index)
{
    int size = py.pts.size();
    list<IP>::iterator it;

    for (int i = 0; i < size; i++)
    {
        Point p1 = py.pts[i];
        IP ip;
        ip.point_flag = 0;
        ip.p = p1;
        comlist.push_back(ip);
        list<IP> oneSeg;
        for (it = iplist.begin(); it != iplist.end(); it++)
        {
            if ((index == 0 && i == it->index0) ||
                (index == 1 && i == it->index1))
            {
                it->dis = get_distance(it->p, p1);
                it->point_flag = 1;
                oneSeg.push_back(*it);
            }
        }
        oneSeg.sort(distance_comparator);
        for (it = oneSeg.begin(); it != oneSeg.end(); it++)
            comlist.push_back(*it);
    }
}

void get_polygon_int_in_out(list<IP> &Pglist, Polygon &plgn1)
{
    bool inFlag;
    list<IP>::iterator it;
    for (it = Pglist.begin(); it != Pglist.end(); it++)
    {
        if (it->point_flag == 0)
        {
            if (is_point_inside_polygon(it->p, plgn1))
                inFlag = true;
            else
                inFlag = false;
        }
        else
        {
            inFlag = !inFlag;
            it->in_flag = inFlag;
        }
    }
}

void get_clip_point_in_out(list<IP> &cliplist, list<IP> &Pglist)
{
    list<IP>::iterator it, it1;
    for (it = cliplist.begin(); it != cliplist.end(); it++)
    {
        if (it->point_flag == 0)
            continue;
        for (it1 = Pglist.begin(); it1 != Pglist.end(); it1++)
        {
            if (it1->point_flag == 0)
                continue;
            if (it->p == it1->p)
                it->in_flag = it1->in_flag;
        }
    }
}

void generate_clip_area(list<IP> &Pglist, list<IP> &cliplist)
{
    list<IP>::iterator it, it1;
    Polygon py;
    Color c = {0.0, 0.0, 1.0};

    for (it = Pglist.begin(); it != Pglist.end(); it++)
        if (it->point_flag == 1 && it->in_flag)
            break;
    py.pts.clear();

    while (true)
    {

        if (it == Pglist.end())
            break;
        py.pts.push_back(it->p);
        for (; it != Pglist.end(); it++)
        {
            if (it->point_flag == 1 && !it->in_flag)
                break;
            py.pts.push_back(it->p);
        }
        for (it1 = cliplist.begin(); it1 != cliplist.end(); it1++)
            if (it1->p == it->p)
                break;

        for (; it1 != cliplist.end(); it1++)
        {
            if (it1->point_flag == 1 && it1->in_flag)
                break;
            py.pts.push_back(it1->p);
        }
        if (py.pts[0] == it1->p)
        {
            py.draw_line(c);
            py.pts.clear();
            for (; it != Pglist.end(); it++)
                if (it->point_flag == 1 && it->in_flag)
                    break;
            continue;
        }
        for (; it != Pglist.end(); it++)
            if (it->p == it1->p)
                break;
    }
}

void weilerAtherton(Polygon &p1, Polygon &p2)
{
    list<IP> iplist, Pglist, cliplist;
    generateIntersectPoints(p1, p2, iplist);

    generate_list(p2, iplist, Pglist, 0);
    generate_list(p1, iplist, cliplist, 1);

    get_polygon_int_in_out(Pglist, p1);

    get_clip_point_in_out(cliplist, Pglist);

    generate_clip_area(Pglist, cliplist);
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, SIZE - 1, 0.0, SIZE - 1);
}

void generate_polygon(Polygon &p, int M)
{
    Point P;
    p.pts.clear();
    for (int i = 0; i < M; ++i)
    {
        bool flag;
        do
        {
            P.x = rand() % SIZE;
            P.y = rand() % SIZE;
            flag = true;
            for (int j = 1; j < i - 1; ++j)
                if (segments_intersect(p.pts[j - 1], p.pts[j], p.pts[i - 1], P))
                {
                    flag = false;
                    break;
                }
            if (flag && i == M - 1)
            {
                for (int j = 2; j < i; ++j)
                    if (segments_intersect(p.pts[j - 1], p.pts[j], P, p.pts[0]))
                    {
                        flag = false;
                        break;
                    }
            }
        } while (!flag);
        p.pts.push_back(P);
    }
}

void Keyboard_action(unsigned char _key, int _x, int _y)
{
    exit(0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_POINT_SMOOTH);

    Polygon plgn1, plgn2;

    plgn1.pts = {{553, 495}, {181, 175}, {486, 71}, {61, 86}, {90, 200}};
    plgn2.pts = {{390, 424}, {579, 585}, {207, 12}, {68, 245}};

    int size = plgn1.pts.size();
    for (int i = 0; i < size; ++i)
        cout << plgn1.pts[i].x << " " << plgn1.pts[i].y << endl;
    cout << endl;
    size = plgn2.pts.size();
    for (int i = 0; i < size; ++i)
        cout << plgn2.pts[i].x << " " << plgn2.pts[i].y << endl;

    Color a = {1.0, 0.0, 0.0};
    Color b = {0.0, 1.0, 0.0};
    plgn2.draw_line(a);
    plgn1.draw_line(b);
    weilerAtherton(plgn1, plgn2);

    glFlush();
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(SIZE, SIZE);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Weiler-Atherton Clipping Algorithm");

    glutKeyboardFunc(Keyboard_action);
    glutDisplayFunc(display);

    init();

    glutMainLoop();

    return 0;
}