#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
 
#define sync std::ios_base::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
using std::vector;
using std::initializer_list;
using std::pair;
using std::make_pair;
using std::cin;
using std::cout;
using std::max;
using std::min;
using std::swap;
using std::setprecision;
 
using ll = long long;
using ldouble = long double;
#define PI 3.141592653589793238463
 
const ldouble epsilon = 1e-7;
bool equal(ldouble first, ldouble second) {
    return fabsl(first - second) <= epsilon;
}
ldouble squaring(ldouble t) {
    return t * t;
}
ldouble deg_to_rad(ldouble angle) {
    return (angle / 180) * PI;
}
bool bit(ll mask, int pos) {
    return (mask >> pos) & 1;
}
 
 
class Line;
 
 
////////////    POINT    ////////////
struct Point {
    ldouble x = 0.0;
    ldouble y = 0.0;
    Point() = default;
    ~Point() = default;
    Point(ldouble x, ldouble y) : x(x), y(y) {}
 
    Point rotate(const Point& center, ldouble angle) const;
    void scale(const Point& center, ldouble k);
    void reflex(const Point& center);
    void reflex(const Line& axis); //определение после Line
};
Point Point::rotate(const Point& center, ldouble angle) const{ // угол в радианах!!
    ldouble x = center.x + cos(angle) * (this->x - center.x) - sin(angle) * (this->y - center.y);
    ldouble y = center.y + sin(angle) * (this->x - center.x) + cos(angle) * (this->y - center.y);
    return Point(x, y);
}
void Point::scale(const Point& center, ldouble k) {
    x = (x - center.x) * k + center.x;
    y = (y - center.y) * k + center.y;
}
void Point::reflex(const Point& center) {
    x = 2 * center.x - x;
    y = 2 * center.y - y;
}
bool operator==(const Point& point1, const Point& point2) {
    return equal(point1.x, point2.x) && equal(point1.y, point2.y);
}
bool operator!=(const Point& point1, const Point& point2) {
    return !(point1 == point2);
}
Point operator+(const Point& point1, const Point& point2) {
    return Point(point1.x + point2.x, point1.y + point2.y);
}
Point operator-(const Point& point1, const Point& point2) {
    return Point(point1.x - point2.x, point1.y - point2.y);
}
Point middle_point(const Point& point1, const Point& point2) {
    return Point((point1.x + point2.x) / 2, (point1.y + point2.y) / 2);
}
ldouble points_distance(const Point& point1, const Point& point2) {
    return sqrtl(squaring(point1.x - point2.x) + squaring(point1.y - point2.y));
}
bool is_lower_left(const Point& point1, const Point& point2) {
    return (point1.y < point2.y) || (equal(point1.y, point2.y) && point1.x < point2.x);
}
ldouble len(const Point& p1, const Point& p2) {
    return squaring(p2.x - p1.x) + squaring(p2.y - p1.y);
}
 
 
//////////    Vector    //////////
class Vector {
public:
    Point radius;
    Vector() = default;
    Vector(ldouble, ldouble, ldouble, ldouble);
    Vector(const Point&, const Point&);
    explicit Vector(const Point&);
 
    ldouble len() const;
};
 
Vector::Vector(ldouble x1, ldouble y1, ldouble x2, ldouble y2) {
    radius = Point(x2 - x1, y2 - y1);
}
Vector::Vector(const Point &p1, const Point &p2) : radius(p2 - p1) {}
 
Vector::Vector(const Point &point) : radius(point) {}
 
ldouble Vector::len() const {
    return sqrtl(squaring(radius.x) + squaring(radius.y));
}
bool operator==(const Vector& vec1, const Vector& vec2) {
    return vec1.radius == vec2.radius;
}
bool operator!=(const Vector& vec1, const Vector& vec2) {
    return !(vec1 == vec2);
}
Vector operator+(const Vector& vec1, const Vector& vec2) {
    return Vector(vec1.radius + vec2.radius);
}
ldouble ScalarMultiply(const Vector& vec1, const Vector& vec2) {
    return vec1.radius.x * vec2.radius.x + vec1.radius.y * vec2.radius.y;
}
ldouble VectorMultiply(const Vector& vec1, const Vector& vec2) {
    return vec1.radius.x * vec2.radius.y - vec2.radius.x * vec1.radius.y;
}
ldouble cos_angle(const Vector& vec1, const Vector& vec2) {
    return ScalarMultiply(vec1, vec2) / (vec1.len() * vec2.len());
}
 
 
//////////    Segment    //////////
class Segment {
public:
    Point p1;
    Point p2;
    Segment() = default;
    Segment(ldouble, ldouble, ldouble, ldouble);
    Segment(const Point&, const Point&);
    bool ContainsPoint(const Point& point) const;
    ldouble len() const;
};
 
Segment::Segment(ldouble x1, ldouble y1, ldouble x2, ldouble y2) {
    p1 = Point(x1, y1);
    p2 = Point(x2, y2);
}
Segment::Segment(const Point &p1, const Point &p2) : p1(p1), p2(p2) {}
 
bool Segment::ContainsPoint(const Point &point) const {
    Vector point_to_first(p1, point);
    Vector point_to_second(p2, point);
    return (VectorMultiply(point_to_first, point_to_second) == 0
            && ScalarMultiply(point_to_first, point_to_second) <= 0);
}
ldouble Segment::len() const {
    return sqrtl(squaring(p2.x - p1.x) + squaring(p2.y - p1.y));
}
 
bool is_intersect(const Segment& seg1, const Segment& seg2) {
    bool intersect = true;
    if (max(seg1.p1.x, seg1.p2.x) < min(seg2.p1.x, seg2.p2.x)) intersect = false;
    if (max(seg2.p1.x, seg2.p2.x) < min(seg1.p1.x, seg1.p2.x)) intersect = false;
    if (max(seg1.p1.y, seg1.p2.y) < min(seg2.p1.y, seg2.p2.y)) intersect = false;
    if (max(seg2.p1.y, seg2.p2.y) < min(seg1.p1.y, seg1.p2.y)) intersect = false;
 
    auto det1 = VectorMultiply(Vector(seg1.p1, seg2.p1), Vector(seg1.p1, seg1.p2));
    auto det2 = VectorMultiply(Vector(seg1.p1, seg2.p2), Vector(seg1.p1, seg1.p2));
    if (det1 * det2 > 0) intersect = false;
 
    det1 = VectorMultiply(Vector(seg2.p1, seg1.p1), Vector(seg2.p1, seg2.p2));
    det2 = VectorMultiply(Vector(seg2.p1, seg1.p2), Vector(seg2.p1, seg2.p2));
    if (det1 * det2 > 0) intersect = false;
 
    return intersect;
}
 
 
////////////    LINE    ////////////
// Ax  + By + C = 0
class Line {
public:
    ldouble a = 0.0;
    ldouble b = 0.0;
    ldouble c = 0.0;
    Line() = default;
    Line(const Point&, const Point&);
    Line(ldouble k, ldouble b) : a(-k), b(1), c(-b) {}
    Line(const Point& point, ldouble k) : a(-k), b(1) {
        c = point.x * k - point.y;
    }
    Line(const ldouble a, const ldouble b, const ldouble c): a(a), b(b), c(c) {};
    ~Line() = default;
    Vector getNormalVector() const;
    Vector getGuidingVector() const;
};
Line::Line(const Point& point1, const Point& point2) {
    a = point1.y - point2.y;
    b = point2.x - point1.x;
    c = point1.x * point2.y - point2.x * point1.y;
}
bool operator==(const Line& line1, const Line& line2) {
    if (equal(line1.a * line2.b, line2.a * line1.b))
        if (equal(line1.b * line2.c, line2.b * line1.c))
            return true;
    return false;
}
bool operator!=(const Line& line1, const Line& line2) {
    return !(line1 == line2);
}
bool is_intersection(const Line& line1, const Line& line2) {
    return !equal(line1.a * line2.b, line1.b * line2.a);
}
bool is_parallel(const Line& line1, const Line& line2) {
    return line1 != line2 && !is_intersection(line1, line2);
}
bool is_collinear(const Line& line1, const Line& line2) {
    return line1 == line2 || !is_intersection(line1, line2);
}
bool is_perpendicular(const Line& line1, const Line& line2) {
    return equal(line1.a * line2.a, -line1.b * line2.b);
}
Point cross(const Line& line1, const Line& line2) {
    Point tmp;
    // метод Крамера
    tmp.x = (line1.b * line2.c - line2.b * line1.c) / (line1.a * line2.b - line2.a * line1.b);
    tmp.y = (line2.a * line1.c - line1.a * line2.c) / (line1.a * line2.b - line2.a * line1.b);
    return tmp;
}
Line perpendicular(const Line& line) {
    return Line(line.b, -line.a, 0);
}
Line middle_perpendicular(const Point& point1, const Point& point2) {
    Point middle = middle_point(point1, point2);
    Line axis(point1, point2);
    Line result = perpendicular(axis);
    result.c = -result.a * middle.x - result.b * middle.y;
    return result;
}
Vector Line::getNormalVector() const {
    return Vector(Point(a, b));
}
 
Vector Line::getGuidingVector() const {
    auto point = Point(-b, a);
    if (equal(point.x, 0)) {
        point.x = 0;
    }
    if (equal(point.y, 0)) {
        point.y = 0;
    }
    return Vector(point);
}
ldouble lines_distance(const Line &line1, const Line &line2) {
    return fabsl(line2.c * (line1.a / line2.a) - line1.c) / sqrtl(line1.a * line1.a + line1.b * line1.b);
}
 
 
void Point::reflex(const Line& axis) {
    Line tmp = perpendicular(axis);
    tmp.c = -tmp.a * x - tmp.b * y;
    scale(cross(tmp, axis), -1);
}
 
 
////////////    SHAPE    ////////////
class Shape {
public:
    virtual ldouble perimeter() const = 0;
    virtual ldouble area() const = 0;
    virtual bool operator==(const Shape& another) const = 0;
    virtual bool operator!=(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape&) const = 0;
    virtual bool isSimilarTo(const Shape&) const = 0;
    virtual bool containsPoint(Point point) const = 0;
 
    virtual void rotate(Point center, ldouble angle) = 0;
    virtual void reflex(Point center) = 0;
    virtual void reflex(Line axis) = 0;
    virtual void scale(Point center, ldouble k) = 0;
    virtual ~Shape() = 0;
};
Shape::~Shape() {}
 

////////////    POLYGON    //////////// 
struct Comp{
    Point base;
    bool operator()(Point a, Point b) {
        auto v1 = Vector(base, a);
        auto v2 = Vector(base, b);
        auto angle = VectorMultiply(v1, v2);
        return !(angle < 0 || (angle == 0 && v1.len() > v2.len()));
    }
};
 
class Polygon : public Shape {
protected:
    vector <Point> vertices;
public:
    Polygon() = default;
    Polygon(const vector<Point>&);
    Polygon(initializer_list<Point>);
    ~Polygon() = default;
    int verticesCount() const;
    vector<Point> getVertices();
    bool isConvex() const;
    Polygon getConvexHull() const;
 
    ldouble perimeter() const override;
    ldouble area() const override;
    bool operator==(const Shape&) const override;
    bool operator!=(const Shape&) const override;
    bool isCongruentTo(const Shape&) const override;
    bool isSimilarTo(const Shape&) const override;
    bool containsPoint(Point point) const override;
    void rotate(Point center, ldouble angle) override;
    void reflex(Point center) override;
    void reflex(Line axis) override;
    void scale(Point center, ldouble coefficient) override;
};
Polygon::Polygon(const vector<Point>& vec) : vertices(vec) {}
Polygon::Polygon(initializer_list<Point> vec) : vertices(vec) {}
int Polygon::verticesCount() const {
    return vertices.size();
}
vector<Point> Polygon::getVertices() {
    return vertices;
}
bool Polygon::isConvex() const {
    bool is_line = true;
    for (int i = 0; i < (int)vertices.size() - 2; ++i) {
        Line line1(vertices[i], vertices[i+1]);
        Line line2(vertices[i+1], vertices[i + 2]);
        if (!is_collinear(line1, line2)) {
            is_line = false;
        }
    }
    if (is_line) return false;
 
    // [ <p_i,p_i+1> , <p_i+1,p_i+2> ]
    bool is_det_found = false;
    int k = 0;
    ldouble determinant = 0;
    Vector v1, v2;
    while (!is_det_found) {
        v1 = Vector(vertices[k], vertices[k + 1]);
        v2 = Vector(vertices[k + 1], vertices[k + 2]);
        determinant = VectorMultiply(v1, v2);
        if (determinant != 0) {
            is_det_found = true;
        }
        ++k;
    }
    bool is_positive;
    if (determinant > 0) {
        is_positive = true;
    }
    else {
        is_positive = false;
    }
    int size = verticesCount();
    for (int i = k; i < size + k; ++i) {
        v1 = Vector(vertices[i % size], vertices[(i + 1) % size]);
        v2 = Vector(vertices[(i + 1) % size], vertices[(i + 2) % size]);
        determinant = VectorMultiply(v1, v2);
        if ((determinant > 0 && !is_positive) || (determinant < 0 && is_positive)) {
            return false;
        }
    }
    return true;
}
Polygon Polygon::getConvexHull() const {
    auto vertices_stack = vertices;
    int n = verticesCount();
    // get left lowest point
    int low_index = 0;
    for (int i = 0; i < n; ++i) {
        if (is_lower_left(vertices_stack[i], vertices_stack[low_index])) {
            low_index = i;
        }
    }
    vector<Point> hull_vertices(n);
    hull_vertices[0] = vertices_stack[low_index];
    swap(vertices_stack[0], vertices_stack[low_index]);
 
    // sort vertices by angle value
    Comp comp;
    comp.base = vertices_stack[0];
    std::sort(vertices_stack.begin(), vertices_stack.end(), comp);
 
    // finding second vertice for hull
    int second_index = 1;
    for (; ; ++second_index) {
        auto v1 = Vector(vertices_stack[0], vertices_stack[second_index + 1]);
        auto v2 = Vector(vertices_stack[0], vertices_stack[1]);
        if (!equal(VectorMultiply(v1, v2), 0)) {
            break;
        }
    }
    hull_vertices[1] = vertices_stack[second_index];
    hull_vertices[2] = vertices_stack[second_index + 1];
 
    int count = 2;
    for (int i = second_index + 2; i < n; ++i) {
        // check if convex
        for (; ; --count) {
            auto v1 = Vector(hull_vertices[count - 1], hull_vertices[count]);
            auto v2 = Vector(hull_vertices[count], vertices_stack[i]);
            if (VectorMultiply(v1, v2) > 0) {
                break;
            }
        }
        ++count;
        // add point to stack
        hull_vertices[count] = vertices_stack[i];
    }
    hull_vertices.resize(count + 1);
    return Polygon(hull_vertices);
}
ldouble Polygon::perimeter() const {
    ldouble P = 0;
    int size = verticesCount();
    for (int i = 0; i < size; ++i) {
        P += points_distance(vertices[i], vertices[(i + 1) % size]);
    }
    return P;
}
ldouble Polygon::area() const {
    ldouble S = 0;
    int size = verticesCount();
    for (int i = 0; i < size; ++i) {
        S += vertices[i].x * vertices[(i + 1) % size].y - vertices[i].y * vertices[(i + 1) % size].x;
    }
    return fabsl(S / 2);
}
bool Polygon::operator==(const Shape& another) const {
    auto polygon_ptr = dynamic_cast<const Polygon*>(&another);
    if (!polygon_ptr) return false;
    if (verticesCount() != polygon_ptr->verticesCount()) return false;
    Point first_point = polygon_ptr->vertices[0];
    int match_index = -1;
    for (int i = 0; i < verticesCount(); ++i) {
        if (vertices[i] == first_point) {
            match_index = i;
            break;
        }
    }
    if (match_index == -1) return false;
    bool is_equal = true;
    // обход в одну сторону
    for(int i = 0; i < verticesCount(); ++i) {
        if (polygon_ptr->vertices[i] != vertices[(match_index + i) % verticesCount()]) {
            is_equal = false;
            break;
        }
    }
    if (is_equal) return true;
    // пробуем обход в другую сторону
    is_equal = true;
    for(int i = 0; i < verticesCount(); ++i) {
        int j;
        if (match_index - i < 0) j = verticesCount() + (match_index - i);
        else j = match_index - i;
        if (polygon_ptr->vertices[i] != vertices[j]) {
            is_equal = false;
            break;
        }
    }
    return is_equal;
}
bool Polygon::operator!=(const Shape& another) const {
    return !(*this == another);
}
bool Polygon::isCongruentTo(const Shape& another) const {
    auto polygon_ptr = dynamic_cast<const Polygon*>(&another);
    if (!polygon_ptr) return false;
    if (verticesCount() != polygon_ptr->verticesCount()) return false;
    vector<ldouble> side_len_this, side_len_that;
    for (int i = 0; i < verticesCount() - 1; ++i) {
        side_len_this.push_back(points_distance(vertices[i], vertices[i + 1]));
        side_len_that.push_back(points_distance(polygon_ptr->vertices[i], polygon_ptr->vertices[i + 1]));
    }
    side_len_this.push_back(points_distance(vertices[verticesCount() - 1], vertices[0]));
    side_len_that.push_back(points_distance(polygon_ptr->vertices[verticesCount() - 1], polygon_ptr->vertices[0]));
    bool is_equal = true;
    int match_index = -1, first_index = 0;
    ldouble first_len = side_len_that[0];
    while (match_index < verticesCount()) {
        for (int i = first_index; i < verticesCount(); ++i) {
            if (equal(side_len_this[i], first_len)) {
                match_index = i;
                break;
            }
        }
        if (match_index == -1) return false;
        // обход в одну сторону
        for(int i = 0; i < verticesCount(); ++i) {
            if (!equal(side_len_that[i],side_len_this[(match_index + i) % verticesCount()])) {
                is_equal = false;
                break;
            }
        }
        if (is_equal) return true;
        // пробуем обход в другую сторону
        is_equal = true;
        for(int i = 0; i < verticesCount(); ++i) {
            int j;
            if (match_index - i < 0) j = verticesCount() + (match_index - i);
            else j = match_index - i;
            if (!equal(side_len_that[i],side_len_this[j])) {
                is_equal = false;
                break;
            }
        }
        if (is_equal) return true;
        first_index = match_index + 1;
        match_index = -1;
    }
    return is_equal;
 
}
bool Polygon::isSimilarTo(const Shape& another) const {
    auto polygon_ptr = dynamic_cast<const Polygon*>(&another);
    if (!polygon_ptr) return false;
    ldouble ratio = perimeter() / polygon_ptr->perimeter();
    return equal(squaring(ratio), area() / polygon_ptr->area());
}
bool Polygon::containsPoint(Point point) const {
    for (int i = 0; i < verticesCount(); ++i) {
        auto seg = Segment(vertices[i], vertices[(i + 1) % verticesCount()]);
        if (seg.ContainsPoint(point)) {
            return true;
        }
    }
    bool is_inside = false;
    int j = verticesCount() - 1;
    for (int i = 0; i < verticesCount(); ++i) {
        // false if there is an even number of sides of poly on the left and true with an odd one.
        bool one_direction = vertices[i].y < point.y && vertices[j].y >= point.y;
        bool other_direction = vertices[j].y < point.y && vertices[i].y >= point.y;
        if (one_direction || other_direction) {
            ldouble ray_poly_cross_x = vertices[i].x + (point.y - vertices[i].y) /
                                                       (vertices[j].y - vertices[i].y) * (vertices[j].x - vertices[i].x);
            if (ray_poly_cross_x < point.x) {
                is_inside = !is_inside;
            }
        }
        j = i;
    }
    return is_inside;
}
void Polygon::rotate(Point center, ldouble angle) {
    angle *= PI / 180;
    for (int i = 0; i < verticesCount(); ++i)
        vertices[i] = vertices[i].rotate(center, angle);
}
void Polygon::reflex(Point center) {
    scale(center, -1);
}
void Polygon::reflex(Line axis) {
    for (int i = 0; i < verticesCount(); ++i)
        vertices[i].reflex(axis);
}
void Polygon::scale(Point center, ldouble k) {
    for (int i = 0; i < verticesCount(); ++i)
        vertices[i].scale(center, k);
}
 
 
 
////////////    ELLIPSE    ////////////
// x^2/a^2 + y^2/b^2 = 1;
class Ellipse : public Shape {
protected:
    ldouble a = 1.0;
    ldouble b = 1.0;
    pair<Point, Point> FF;
public:
    Ellipse() = default;
    Ellipse(const Point&, const Point&, ldouble);
    ~Ellipse() = default;
    pair<Point,Point> focuses() const;
    pair<Line, Line> directrices() const;
    ldouble eccentricity() const;
    Point center() const;
 
    ldouble perimeter() const override;
    ldouble area() const override;
    bool operator==(const Shape&) const override;
    bool operator!=(const Shape&) const override;
    bool isCongruentTo(const Shape&) const override;
    bool isSimilarTo(const Shape&) const override;
    bool containsPoint(Point point) const override;
    void rotate(Point center, ldouble angle) override;
    void reflex(Point center) override;
    void reflex(Line axis) override;
    void scale(Point center, ldouble k) override;
};
Ellipse::Ellipse(const Point& point1, const Point& point2, ldouble sum_focal) {
    FF = make_pair(point1, point2);
    a = sum_focal / 2;
    b = sqrtl(squaring(a) - squaring(points_distance(point1, point2) / 2));
}
pair<Point,Point> Ellipse::focuses() const {
    return FF;
}
pair<Line, Line> Ellipse::directrices() const {
    Line axis(FF.first, FF.second);
    std::pair <Line, Line> result {perpendicular(axis), perpendicular(axis)};
    result.first.c = a * a / sqrtl(a * a - b * b) * sqrtl(squaring(result.first.a) + squaring(result.first.b))
                     - result.first.a * center().x - result.first.b * center().y;
    result.second.c = - a * a / sqrtl(a * a - b * b) * sqrtl(squaring(result.second.a) + squaring(result.second.b))
                      - result.second.a * center().x - result.second.b * center().y;
    return result;
}
ldouble Ellipse::eccentricity() const {
    return sqrtl(1 - squaring(b)/squaring(a));
}
Point Ellipse::center() const {
    return middle_point(FF.first, FF.second);
}
ldouble Ellipse::perimeter() const {
    return PI * (3 * (a + b) - sqrtl((3 * a + b) * (a + 3 * b)));
}
ldouble Ellipse::area() const {
    return PI * a * b;
}
bool Ellipse::operator==(const Shape& another) const {
    auto ellipse_ptr = dynamic_cast<const Ellipse*>(&another);
    if (!ellipse_ptr) return false;
    if ((FF.first == ellipse_ptr->FF.first && FF.second == ellipse_ptr->FF.second) && equal(a, ellipse_ptr->a))
        return true;
    return (FF.first == ellipse_ptr->FF.second && FF.second == ellipse_ptr->FF.first) && equal(a, ellipse_ptr->a);
}
bool Ellipse::operator!=(const Shape& another) const {
    return !(*this == another);
}
bool Ellipse::isCongruentTo(const Shape& another) const {
    auto ellipse_ptr = dynamic_cast<const Ellipse*>(&another);
    if (!ellipse_ptr) return false;
    return equal(points_distance(FF.first, FF.second), points_distance(ellipse_ptr->FF.first, ellipse_ptr->FF.first)) &&
           equal(a, ellipse_ptr->a);
}
bool Ellipse::isSimilarTo(const Shape& another) const {
    auto ellipse_ptr = dynamic_cast<const Ellipse*>(&another);
    if (!ellipse_ptr) return false;
    return equal(eccentricity(), ellipse_ptr->eccentricity());
}
bool Ellipse::containsPoint(Point point) const {
    ldouble sum_focal = points_distance(FF.first, point) + points_distance(FF.second, point);
    return sum_focal < a * 2 || equal(sum_focal, 2 * a);
}
void Ellipse::rotate(Point center, ldouble angle) {
    angle *= PI / 180 ;
    FF.first = FF.first.rotate(center, angle);
    FF.second = FF.second.rotate(center, angle);
}
void Ellipse::reflex(Point center) {
    scale(center, -1);
}
void Ellipse::reflex(Line axis) {
    FF.first.reflex(axis);
    FF.second.reflex(axis);
}
void Ellipse::scale(Point center, ldouble k) {
    FF.first.scale(center, k);
    FF.second.scale(center, k);
    a *= fabsl(k);
    b *= fabsl(k);
}
 
 
////////////    CIRCLE    ////////////
class Circle : public Ellipse {
public:
    Circle() : Ellipse() {};
    Circle(const Point&, ldouble);
    ~Circle() = default;
    ldouble radius() const;
};
Circle::Circle(const Point& center, ldouble radius) : Ellipse(center, center, 2 * radius) {}
ldouble Circle::radius() const {
    return a;
}
 
 
////////////    RECTANGLE    ////////////
class Rectangle : public Polygon {
public:
    Rectangle() : Polygon() {};
    Rectangle(const Point&, const Point&, ldouble k);
    ~Rectangle() = default;
    Point center() const;
    pair<Line, Line> diagonals() const;
};
Rectangle::Rectangle(const Point& point1, const Point& point2, ldouble k) : Polygon{point1, Point(0, 0), point2, Point(0, 0)} {
    ldouble angle = 180 - 2 * atan(k) * 180 / PI;
    Point middle = middle_point(point1, point2);
    vertices[1] = point1.rotate(middle, angle);
    vertices[3] = point2.rotate(middle, angle);
}
Point Rectangle::center() const {
    return middle_point(vertices[0], vertices[2]);
}
pair<Line, Line> Rectangle::diagonals() const {
    return {Line(vertices[0], vertices[2]), Line(vertices[1], vertices[3])};
}
 
 
////////////    SQUARE    ////////////
class Square : public Rectangle {
public:
    Square() : Rectangle() {};
    Square(const Point&, const Point&);
    ~Square() = default;
    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;
};
Square::Square(const Point& point1, const Point& point2) : Rectangle(point1, point2, 1) {}
Circle Square::circumscribedCircle() const {
    return Circle(middle_point(vertices[0], vertices[2]),
                  points_distance(vertices[0], vertices[2]) / 2);
}
Circle Square::inscribedCircle() const {
    return Circle(middle_point(vertices[0], vertices[2]),
                  points_distance(vertices[0], vertices[1]) / 2);
}
 
////////////    TRIANGLE    ////////////
class Triangle : public Polygon {
public:
    Triangle() : Polygon() {};
    Triangle(const Point&, const Point&, const Point&);
    ~Triangle() = default;
    Circle circumscribedCircle();
    Circle inscribedCircle();
    Point centroid();
    Point orthocenter();
    Line EulerLine();
    Circle ninePointsCircle();
};
Triangle::Triangle(const Point& point1, const Point& point2, const Point& point3) : Polygon{point1, point2, point3} {}
Circle Triangle::circumscribedCircle() {
    Line mp1 = middle_perpendicular(vertices[0], vertices[1]);
    Line mp2 = middle_perpendicular(vertices[1], vertices[2]);
    return Circle(cross(mp1, mp2), points_distance(cross(mp1, mp2), vertices[0]));
}
Circle Triangle::inscribedCircle() {
    ldouble a = points_distance(vertices[1], vertices[2]);
    ldouble b = points_distance(vertices[2], vertices[0]);
    ldouble c = points_distance(vertices[0], vertices[1]);
    Point tmp;
    tmp.x = (vertices[0].x * a + vertices[1].x * b + vertices[2].x * c) / (a + b + c);
    tmp.y = (vertices[0].y * a + vertices[1].y * b + vertices[2].y * c) / (a + b + c);
    return Circle(tmp, 2 * area() / perimeter());
}
Point Triangle::centroid() {
    ldouble x = (vertices[0].x + vertices[1].x + vertices[2].x) / 3;
    ldouble y = (vertices[0].y + vertices[1].y + vertices[2].y) / 3;
    return Point(x, y);
}
Point Triangle::orthocenter() {
    Line ha = perpendicular(Line(vertices[0], vertices[1]));
    ha.c = -ha.a * vertices[2].x - ha.b * vertices[2].y;
    Line hb = perpendicular(Line(vertices[1], vertices[2]));
    hb.c = -hb.a * vertices[0].x - hb.b * vertices[0].y;
    return cross(ha, hb);
}
Line Triangle::EulerLine() {
    return Line(circumscribedCircle().center(), orthocenter());
}
 
Circle Triangle::ninePointsCircle() {
    Point center = middle_point(circumscribedCircle().center(), orthocenter());
    return Circle(center, circumscribedCircle().radius()/2);
}
