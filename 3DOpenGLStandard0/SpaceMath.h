#pragma once
#include <math.h>
#include <string>
#include <vector>
#include <list>

using namespace std;

#define inside(A, C, B) (A < C && C < B)

namespace shp {

	const float PI = 3.141592f;

	//구조체들

	// 점
	class vec2f {
	public:
		float x;
		float y;

		vec2f() { x = 0; y = 0; }
		vec2f(const vec2f& ref) { x = ref.x; y = ref.y; }
		vec2f(float ix, float iy) { x = ix; y = iy; }

		vec2f operator+(vec2f v2) { return vec2f(x + v2.x, y + v2.y); }
		vec2f operator-(vec2f v2) { return vec2f(x - v2.x, y - v2.y); }
		vec2f operator/(float div) { return vec2f(x / div, y / div); }
		vec2f operator*(float div) { return vec2f(x * div, y * div); }
		bool operator==(const vec2f& v2) {
			if (x == v2.x && y == v2.y) return true;
			else return false;
		}
		bool operator!=(const vec2f& v2) {
			if (x != v2.x || y != v2.y) return true;
			else return false;
		}

		vec2f Inverse() const { return vec2f(-x, -y); }

		bool isActive() const {
			if ((isnan(x) == false && isinf(x) == false) && (isnan(y) == false && isinf(y) == false)) {
				return true;
			}
			else return false;
		}
	};

	class vec3f {
	public:
		float x;
		float y;
		float z;

		vec3f() { x = 0; y = 0; }
		vec3f(const vec3f& ref) { x = ref.x; y = ref.y; z = ref.z; }
		vec3f(float ix, float iy, float iz) { x = ix; y = iy; z = iz; }

		vec3f operator+(vec3f v2) { return vec3f(x + v2.x, y + v2.y, z + v2.z); }
		vec3f operator-(vec3f v2) { return vec3f(x - v2.x, y - v2.y, z - v2.z); }
		vec3f operator/(float div) { return vec3f(x / div, y / div, z / div); }
		vec3f operator*(float div) { return vec3f(x * div, y * div, z * div); }
		bool operator==(const vec3f& v2) {
			if ((x == v2.x && y == v2.y) && z == v2.z) return true;
			else return false;
		}
		bool operator!=(const vec3f& v2) {
			if ((x != v2.x || y != v2.y) || z != v2.z) return true;
			else return false;
		}

		vec3f Inverse() const { return vec3f(-x, -y, -z); }

		bool isActive() const {
			if (((isnan(x) == false && isinf(x) == false) && (isnan(y) == false && isinf(y) == false))
				&& (isnan(z) == false && isinf(z) == false)) {
				return true;
			}
			else return false;
		}
	};

	// 직선
	class straightLine {
	public:
		float xrate; // x 변화량
		float yrate; // y 변화량
		vec2f inDot; // 선 안의 하나의 점.
		straightLine() { xrate = 0; yrate = 0; inDot = vec2f(); }
		straightLine(const straightLine& ref) { xrate = ref.xrate; yrate = ref.yrate; inDot = ref.inDot; }
		straightLine(float xr, float yr, vec2f indot) { xrate = xr; yrate = yr; inDot = indot; }
		straightLine(vec2f p1, vec2f p2) { xrate = p2.x - p1.x; yrate = p2.y - p1.y; inDot = vec2f(p1); }

		float GetYFromX(float x) const { return yrate * (x - inDot.x) / xrate + inDot.y; }
		float GetXFromY(float y) const { return xrate * (y - inDot.y) / yrate + inDot.x; }

		bool operator==(const straightLine& ref) {
			if (GetYFromX(0) == ref.GetYFromX(0) && GetXFromY(0) == ref.GetXFromY(0)) {
				return true;
			}
			else return false;
		}

		void turnRight() {
			float d = xrate;
			xrate = yrate;
			yrate = -d;
		}
	};

	// 직사각형
	class rect4f {
	private:
		void moveValue_tool1(char xy, float value, float mulminv, int plus) {
			float save;
			if (xy == 'x') {
				save = abs(lx - fx);
				fx = value - save * mulminv;
				lx = fx + save * plus;
			}
			else if (xy == 'y') {
				save = abs(ly - fy);
				fy = value - save * mulminv;
				ly = fy + save * plus;
			}
		}
	public:
		float fx;
		float fy;
		float lx;
		float ly;

		rect4f() { fx = 0; fy = 0; lx = 0; ly = 0; }
		rect4f(float FX, float FY, float LX, float LY) { fx = FX; fy = FY; lx = LX; ly = LY; }
		rect4f(const rect4f& ref) { fx = ref.fx; fy = ref.fy; lx = ref.lx; ly = ref.ly; }
		virtual ~rect4f() {}

		bool operator==(const rect4f& v2) {
			if ((v2.fx == fx && v2.fy == fy)
				&& v2.lx == lx && v2.ly == ly) return true;
			else return false;
		}

		vec2f getCenter() const { return (vec2f(fx, fy) + vec2f(lx, ly)) / 2; }
		float getw() const { return lx - fx; }
		float geth() const { return ly - fy; }

		void setCenter(const vec2f& pos) {
			float dx = abs(lx - fx);
			float dy = abs(ly - fy);
			fx = pos.x - dx / 2;
			fy = pos.y - dy / 2;
			lx = pos.x + dx / 2;
			ly = pos.y + dy / 2;
		}

		void moveValue(const string& name, float value) {
			float save;
			if (name.at(0) == 'f') {
				moveValue_tool1(name.at(1), value, 0, 1);
			}
			else if (name.at(0) == 'l') {
				moveValue_tool1(name.at(1), value, 0, -1);
				if (name.at(1) == 'x') {
					float a = lx;
					lx = fx;
					fx = a;
				}
				else {
					float a = ly;
					ly = fy;
					fy = a;
				}
			}
			else if (name.at(0) == 'c') {
				moveValue_tool1(name.at(1), value, 0.5f, 1);
			}
		}
	};

	// 직육면체
	class cube6f {
	public:
		float fx;
		float fy;
		float fz;
		float lx;
		float ly;
		float lz;

		cube6f() { fx = 0; fy = 0; lx = 0; ly = 0; fz = 0; lz = 0; }
		cube6f(float FX, float FY, float FZ, float LX, float LY, float LZ) { fx = FX; fy = FY; fz = FZ; lx = LX; ly = LY; lz = LZ; }
		cube6f(const cube6f& ref) { fx = ref.fx; fy = ref.fy; fz = ref.fz; lx = ref.lx; ly = ref.ly; lz = ref.lz; }
		virtual ~cube6f() {}

		bool operator==(const cube6f& v2) {
			if (((v2.fx == fx && v2.fy == fy) && v2.fz == fz)
				&& ((v2.lx == lx && v2.ly == ly) && v2.lz == lz))
				return true;
			else return false;
		}

		bool operator!=(const cube6f& v2) {
			if (((v2.fx != fx || v2.fy != fy) || v2.fz != fz)
				&& ((v2.lx != lx || v2.ly != ly) || v2.lz != lz))
				return true;
			else return false;
		}

		vec3f getCenter() const { return (vec3f(fx, fy, fz) + vec3f(lx, ly, lz)) / 2; }
		float getw() const { return lx - fx; }
		float geth() const { return ly - fy; }
		float getd() const { return lz - fz; }

		void setCenter(const vec3f& pos) {
			float dx = abs(lx - fx);
			float dy = abs(ly - fy);
			float dz = abs(lz - fz);
			fx = pos.x - dx / 2;
			fy = pos.y - dy / 2;
			fz = pos.z - dz / 2;
			lx = pos.x + dx / 2;
			ly = pos.y + dy / 2;
			lz = pos.z + dz / 2;
		}
	};

	// 각도 - 방향
	class angle2f {
	public:
		float dx;
		float dy;
		float radian;

		angle2f() { dx = 0; dy = 0; radian = 0; }
		angle2f(const angle2f& ref) { dx = ref.dx; dy = ref.dy; radian = ref.radian; }
		angle2f(float ix, float iy) { *this = usedxdy(ix, iy); }
		angle2f(float angle, bool radian) {
			if (radian) {
				*this = useRadian(angle);
			}
			else {
				*this = useNumAngle(angle);
			}
		}
		angle2f& operator= (const angle2f& ref)
		{
			dx = ref.dx;
			dy = ref.dy;
			radian = ref.radian;
			return *this;
		}

		angle2f operator+(const angle2f& v2) {
			return useRadian(radian + v2.radian);
		}
		angle2f operator-(const angle2f& v2) { return useRadian(radian - v2.radian); }
		//angle2f operator/(angle2f div) { return vec2f(x / div, y / div); }
		//angle2f operator*(angle2f div) { return vec2f(x * div, y * div); }
		bool operator==(const angle2f& v2) {
			if (radian == v2.radian) return true;
			else return false;
		}
		bool operator!=(const angle2f& v2) {
			if (radian != v2.radian) return true;
			else return false;
		}

		angle2f Inverse() const { return useRadian(radian + PI); }

		bool isActive() const {
			if ((isnan(dx) == false && isinf(dx) == false) && (isnan(dy) == false && isinf(dy) == false)) {
				return true;
			}
			else return false;
		}

		static angle2f useRadian(float radian);
		static angle2f usedxdy(float dx, float dy);
		static angle2f useNumAngle(float numangle);
	};

	// 삼각형
	class triangle3v {
	public:
		vec2f point[3];
		triangle3v() {
			for (int i = 0; i < 3; ++i) {
				point[i] = vec2f(0, 0);
			}
		}
		triangle3v(const triangle3v& ref) {
			for (int i = 0; i < 3; ++i) {
				point[i] = ref.point[i];
			}
		}
		triangle3v(vec2f v0, vec2f v1, vec2f v2) {
			point[0] = v0;
			point[1] = v1;
			point[2] = v2;
		}

		bool operator==(const triangle3v& v2) {
			int notconsider[3] = { 0, 0, 0 };
			int considerN = 0;
			for (int i = 0; i < 3; ++i) {
				for (int k = 0; k < 3; ++k) {
					bool out = false;
					for (int u = 0; u < considerN; ++u) {
						if (k == notconsider[u]) {
							out = true;
						}
					}
					if (out) {
						continue;
					}

					if (point[i] == v2.point[k]) {
						notconsider[considerN] = k;
						++considerN;
					}
				}
			}

			if (considerN >= 2) {
				return true;
			}
			else return false;
		}
		bool operator!=(const triangle3v& v2) {
			if (*this == v2) {
				return false;
			}
			else {
				return true;
			}
		}
	};

	// 두점 사이의 거리
	float get_distance(vec2f v1, vec2f v2);

	// 두점 사이의 거리
	float get_distance3D(vec3f v1, vec3f v2);

	// 하나의 선을 기준으로 한 점이 해당 외치에 있는지 확인
	bool bPointInStraightRange(vec2f point, straightLine sl, vec2f dir);

	// 선에 대한 점의 위치
	vec2f GetPointRangeInStraight(vec2f point, straightLine sl);

	// 선에 대한 삼각형의 위치 : (0, 0)이면 곂치는 것.
	vec2f GetTriangleRangeInStraight(triangle3v tri, straightLine s1);

	// 두선의 교점
	vec2f GetCrossPoint(straightLine sl1, straightLine sl2);

	// 한 점을 중심점 기준으로 회전
	vec2f GetRotatePos(vec2f center, vec2f pos, angle2f angle);

	// 두 직선과 같은 각도를 이루는 두개의 직선중 inpos의 영역에 있는 직선을 반환한다.
	straightLine GetSameAngleLine(straightLine s1, straightLine s2, vec2f inpos);

	// 점의 삼각형 내부 위치에 있는지 여부
	bool bPointInTriangleRange(vec2f point, vec2f v1, vec2f v2, vec2f v3);

	// 선의 삼각형 내부 위치에 있는지 여부
	bool bLineInTriangleRange(vec2f linep1, vec2f linep2, vec2f v1, vec2f v2, vec2f v3);

	// 점이 다각형 내부에 있는지 여부
	bool bPointInPolygonRange(vec2f point, vector<vec2f> polygon);

	// 삼각형이 다각형 내부에 있는지 여부
	bool bTriangleInPolygonRange(vector<vec2f> triangle, vector<vec2f> polygon);

	// 점이 직사각형 내부에 있는지 여부
	bool bPointInRectRange(vec2f point, rect4f rt);

	// 직사각형이 직사각형 내부에 있는지 여부
	bool bRectInRectRange(rect4f smallrt, rect4f bigrt, bool include_same, bool include_any_contact);

	// 오목다각형들을 여러 삼각형으로 변환
	vector<triangle3v> FreePolygonToTriangles(vector<vec2f> freepolygon);

	// 삼각형을 선으로 자를때 남겨진 도형을 여러 삼각형들로 변환
	vector<triangle3v> TriangleCutStraightLine(triangle3v triangle, straightLine cutline, vec2f remaindir);

	// 여러 삼각형을 선으로 자를때 남겨진 도형을 여러 삼각형들로 변환
	vector<triangle3v> TrianglesCutStraightLine(vector<triangle3v> triangles, straightLine cutline, vec2f remaindir);

	// 다각형을 직사각형 틀에 담아 자른다.
	vector<triangle3v> FreePolygonInRectRange(vector<vec2f> freepolygon, rect4f rt);

	bool CollidCondition(vec2f rangeA, vec2f rangeB);

	bool isCubeContactCube(cube6f c1, cube6f c2);

	float GetContactLocation(shp::vec2f pv1, shp::vec2f pv2);
};


float shp::get_distance(vec2f v1, vec2f v2) {
	float delta[2];
	delta[0] = v2.x - v1.x;
	delta[1] = v2.y - v1.y;
	return sqrtf(delta[0] * delta[0] + delta[1] * delta[1]);
}

float shp::get_distance3D(vec3f v1, vec3f v2)
{
	float delta[3];
	delta[0] = v2.x - v1.x;
	delta[1] = v2.y - v1.y;
	delta[2] = v2.z - v1.z;
	return sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
}

shp::angle2f shp::angle2f::useRadian(float radian)
{
	angle2f a = angle2f();
	if (radian > 2.0f * PI) {
		int a = radian / (2.0f * PI);
		radian = radian - 2.0f * (float)a * PI;
	}
	if (radian < 0) {
		int a = -radian / (2.0f * PI);
		radian = ((float)(a + 1) * 2.0f * PI) - radian;
	}
	a.radian = radian;
	a.dx = cosf(radian);
	a.dy = sinf(radian);
	return a;
}

shp::angle2f shp::angle2f::useNumAngle(float numangle) {
	return useRadian(shp::PI * numangle / 180.0f);
}

shp::angle2f shp::angle2f::usedxdy(float dx, float dy)
{
	angle2f a = angle2f();
	float l = get_distance(vec2f(0, 0), vec2f(dx, dy));
	dx = dx / l;
	dy = dy / l;
	float angle = (asinf(fabsf(dy)) + acosf(fabsf(dx))) / 2.0f;

	if (dy > 0) {
		if (dx > 0) {
			a.radian = angle;
		}
		else {
			a.radian = PI - angle;
		}
	}
	else {
		if (dx > 0) {
			a.radian = 2 * PI - angle;
		}
		else {
			a.radian = PI + angle;
		}
	}
	a.dx = dx;
	a.dy = dy;
	return a;
}

bool shp::bPointInStraightRange(vec2f point, straightLine sl, vec2f dir)
{
	float x2 = sl.xrate * (point.y - sl.inDot.y) / sl.yrate + sl.inDot.x;
	float y2 = sl.yrate * (point.x - sl.inDot.x) / sl.xrate + sl.inDot.y;
	x2 = x2 - point.x;
	y2 = y2 - point.y;
	if (x2 * dir.x > 0 && y2 * dir.y > 0)
		return true;
	else return false;
}

bool shp::bPointInTriangleRange(vec2f point, vec2f v1, vec2f v2, vec2f v3)
{
	int k = 0;
	straightLine l = straightLine(v1, v2);
	vec2f dir1 = GetPointRangeInStraight(v3, l);
	vec2f dir2 = GetPointRangeInStraight(point, l);
	if (dir1 == dir2) k++;
	else return false;

	l = straightLine(v1, v3);
	dir1 = GetPointRangeInStraight(v2, l);
	dir2 = GetPointRangeInStraight(point, l);
	if (dir1 == dir2) k++;
	else return false;

	l = straightLine(v2, v3);
	dir1 = GetPointRangeInStraight(v1, l);
	dir2 = GetPointRangeInStraight(point, l);
	if (dir1 == dir2) k++;
	else return false;

	if (k == 3) return true;
	else return false;
}

bool shp::bLineInTriangleRange(vec2f linep1, vec2f linep2, vec2f v1, vec2f v2, vec2f v3) {
	straightLine l = straightLine(linep1, linep2);
	vec2f cross[3];
	int k = 0;

	//v의 선중 같은 선분이 있을 경우 false 함

	cross[k] = GetCrossPoint(l, straightLine(v1, v2));
	if (cross[k].isActive()) {
		if (((v1.x <= cross[k].x && cross[k].x <= v2.x) || (v2.x <= cross[k].x && cross[k].x <= v1.x))
			|| ((v1.y <= cross[k].y && cross[k].y <= v2.y) || (v2.y <= cross[k].y && cross[k].y <= v1.y))) {
			k++;
		}
	}

	cross[k] = GetCrossPoint(l, straightLine(v1, v3));
	if (cross[k].isActive()) {
		if (((v1.x <= cross[k].x && cross[k].x <= v3.x) || (v3.x <= cross[k].x && cross[k].x <= v1.x))
			|| ((v1.y <= cross[k].y && cross[k].y <= v3.y) || (v3.y <= cross[k].y && cross[k].y <= v1.y))) {
			k++;
		}
	}

	cross[k] = GetCrossPoint(l, straightLine(v2, v3));
	if (cross[k].isActive()) {
		if (((v2.x <= cross[k].x && cross[k].x <= v3.x) || (v3.x <= cross[k].x && cross[k].x <= v2.x))
			|| ((v2.y <= cross[k].y && cross[k].y <= v3.y) || (v3.y <= cross[k].y && cross[k].y <= v2.y))) {
			k++;
		}
	}

	if (k >= 2) {
		for (int i = 0; i < k; i++) {
			for (int j = 0; j < k; i++) {
				if (j == i) continue;
				if (cross[i] != cross[j]) return true;
			}
		}
		return false;
	}
	else return false;
}

bool shp::bPointInPolygonRange(vec2f point, vector<vec2f> polygon) {
	int h = 0;

	for (int o = 0; o < polygon.size(); o++) {
		vec2f p1;
		vec2f p2;
		if (o == polygon.size() - 1) {
			p1 = polygon.at(o);
			p2 = polygon.at(0);
		}
		else {
			p1 = polygon.at(o);
			p2 = polygon.at(o + 1);
		}

		straightLine sl = straightLine(p1, p2);
		float xx = sl.GetXFromY(point.y);
		if (point.x < xx &&
			((p1.y < point.y && point.y < p2.y) || (p2.y < point.y && point.y < p1.y))) {
			h++;
		}
	}

	if (h % 2 == 1) {
		return true;
	}
	else return false;
}

bool shp::bTriangleInPolygonRange(vector<vec2f> triangle, vector<vec2f> polygon)
{
	if (triangle.size() != 3) return false;
	vec2f gcenter = (triangle.at(0) + triangle.at(1) + triangle.at(2)).operator/(3);
	if (bPointInPolygonRange(gcenter, polygon) == false) {
		return false;
	}
	for (int i = 0; i < 3; i++) {
		vec2f considerP = (gcenter + triangle.at(0) * 3) / 4;
		if (bPointInPolygonRange(considerP, polygon) == false) {
			return false;
		}
	}

	return true;
}

bool shp::bPointInRectRange(vec2f point, rect4f rt)
{
	if ((rt.fx < point.x && point.x < rt.lx)
		&& (rt.fy < point.y && point.y < rt.ly)) {
		return true;
	}
	else return false;
}

bool shp::bRectInRectRange(rect4f smallrt, rect4f bigrt, bool include_same, bool include_any_contact)
{
	int xn = 0;
	int yn = 0;
	if (include_same) {
		if (bigrt.fx <= smallrt.fx && smallrt.fx <= bigrt.lx) xn++;
		if (bigrt.fx <= smallrt.lx && smallrt.lx <= bigrt.lx) xn++;
		if (bigrt.fy <= smallrt.fy && smallrt.fy <= bigrt.ly) yn++;
		if (bigrt.fy <= smallrt.ly && smallrt.ly <= bigrt.ly) yn++;
	}
	else {
		if (bigrt.fx < smallrt.fx && smallrt.fx < bigrt.lx) xn++;
		if (bigrt.fx < smallrt.lx && smallrt.lx < bigrt.lx) xn++;
		if (bigrt.fy < smallrt.fy && smallrt.fy < bigrt.ly) yn++;
		if (bigrt.fy < smallrt.ly && smallrt.ly < bigrt.ly) yn++;
	}

	if (include_any_contact) {
		if (xn > 0 && yn > 0) return true;
		else return false;
	}
	else {
		if (xn > 1 && yn > 1) return true;
		else return false;
	}
}

shp::vec2f shp::GetPointRangeInStraight(vec2f point, straightLine sl)
{
	float x2 = sl.xrate * (point.y - sl.inDot.y) / sl.yrate + sl.inDot.x;
	float y2 = sl.yrate * (point.x - sl.inDot.x) / sl.xrate + sl.inDot.y;
	x2 = point.x - x2;
	y2 = point.y - y2;
	vec2f rv = vec2f(x2 / abs(x2), y2 / abs(y2));
	if (isnan<float>(rv.x)) rv.x = 0;
	if (isnan<float>(rv.y)) rv.y = 0;
	return rv;
}

shp::vec2f shp::GetTriangleRangeInStraight(triangle3v tri, straightLine s1)
{
	vec2f dir[3];
	for (int i = 0; i < 3; ++i) {
		dir[i] = shp::GetPointRangeInStraight(tri.point[i], s1);
	}

	if (dir[0] == dir[1] && dir[1] == dir[2]) {
		return dir[0];
	}
	else {
		return vec2f(0, 0);
	}
}

shp::vec2f shp::GetCrossPoint(straightLine sl1, straightLine sl2)
{
	vec2f cross;
	if (sl1.xrate * sl2.xrate != 0 || sl1.yrate * sl2.yrate != 0) {
		cross.x = (sl1.inDot.x * sl1.yrate / sl1.xrate - sl1.inDot.y - sl2.inDot.x * sl2.yrate / sl2.xrate + sl2.inDot.y) / (sl1.yrate / sl1.xrate - sl2.yrate / sl2.xrate);
		cross.y = sl1.GetYFromX(cross.x);
	}
	else {
		if (sl1.xrate == 0 || sl2.xrate == 0) {
			cross.x = (sl1.xrate == 0) ? sl1.inDot.x : sl2.inDot.x;
			if (sl1.yrate == 0 || sl2.yrate == 0) {
				cross.y = (sl1.yrate == 0) ? sl1.inDot.y : sl2.inDot.y;
			}
			else {
				cross.y = (sl1.xrate == 0) ? sl2.GetYFromX(cross.x) : sl1.GetYFromX(cross.x);
			}
		}
		else {
			cross.y = (sl1.yrate == 0) ? sl1.inDot.y : sl2.inDot.y;
			cross.x = (sl1.yrate == 0) ? sl2.GetXFromY(cross.y) : sl1.GetXFromY(cross.y);
		}

	}
	return cross;
}

shp::vec2f shp::GetRotatePos(vec2f center, vec2f pos, angle2f angle)
{
	float length = get_distance(center, pos);
	vec2f wh = pos - center;
	angle2f rAngle = angle + angle2f::usedxdy(wh.x, wh.y);
	vec2f rPos = vec2f(center.x + rAngle.dx * length, center.y + rAngle.dy * length);
	return rPos;
}

shp::straightLine shp::GetSameAngleLine(straightLine s1, straightLine s2, vec2f inpos)
{
	straightLine rs;
	vec2f cross = GetCrossPoint(s1, s2);
	vec2f dir[2];
	dir[0].x = cosf((atanf(s1.yrate / s1.xrate) + atanf(s2.yrate / s2.xrate)) / 2);
	dir[0].y = sinf((atanf(s1.yrate / s1.xrate) + atanf(s2.yrate / s2.xrate)) / 2);
	dir[1].x = dir[0].y;
	dir[1].y = -dir[0].x;

	vec2f d1 = GetPointRangeInStraight(inpos, s1);
	vec2f d2 = GetPointRangeInStraight(inpos, s2);

	vec2f d3 = GetPointRangeInStraight(cross + dir[0], s1);
	vec2f d4 = GetPointRangeInStraight(cross + dir[0], s2);
	if ((d1 == d3 && d2 == d4) || d1.Inverse() == d3 && d2.Inverse() == d4) {
		rs.xrate = dir[0].x;
		rs.yrate = dir[0].y;
		rs.inDot = cross;
	}
	else {
		rs.xrate = dir[1].x;
		rs.yrate = dir[1].y;
		rs.inDot = cross;
	}
	return rs;
}

vector<shp::triangle3v> shp::FreePolygonToTriangles(vector<vec2f> freepolygon)
{
	list<vec2f> lt;
	vector<triangle3v> triangles;
	for (int i = 0; i < freepolygon.size(); ++i) {
		lt.push_back(freepolygon[i]);
	}

	int siz = lt.size();
	while (lt.size() >= 3) {
		list<vec2f>::iterator lti = lt.begin();
		for (int i = 0; i < lt.size() - 2; i++) {

			bool bdraw = true;
			list<vec2f>::iterator ltk = lt.begin();
			for (int k = 0; k < lt.size(); k++) {
				if (i <= k && k <= i + 2) {
					++ltk;
					continue;
				}

				list<vec2f>::iterator inslti = lti;
				vec2f kv = *ltk;
				vec2f iv[3] = { *inslti , *(++inslti) , *(++inslti) };

				if (bPointInTriangleRange(kv, iv[0], iv[1], iv[3])) {
					bdraw = false;
					break;
				}
			}

			if (bdraw == true) {
				list<vec2f>::iterator inslti = lti;
				vec2f pi = *inslti;
				vec2f pi1 = *(++inslti);
				vec2f pi2 = *(++inslti);
				vec2f gcenter = vec2f((pi.x + pi1.x + pi2.x) / 3, (pi.y + pi1.y + pi2.y) / 3);
				vector<vec2f> tri;
				tri.resize(3); tri[0] = pi; tri[1] = pi1; tri[2] = pi2;
				if (bTriangleInPolygonRange(tri, freepolygon)) {
					triangle3v tri;
					tri.point[0] = pi;
					tri.point[1] = pi1;
					tri.point[2] = pi2;
					triangles.push_back(tri);

					list<vec2f>::iterator inslti2 = lti;
					lt.erase(++inslti);
				}
			}

			++lti;
		}

		if (siz == lt.size()) {
			break;
		}
		else {
			siz = lt.size();
		}

	}
	return triangles;
}

vector<shp::triangle3v> shp::TriangleCutStraightLine(triangle3v triangle, straightLine cutline, vec2f remaindir)
{
	vector<triangle3v> returnTri;
	vec2f dir[3];
	for (int i = 0; i < 3; ++i) {
		dir[i] = shp::GetPointRangeInStraight(triangle.point[i], cutline);
	}

	int count[3] = { 0, 0, 0 };
	int cnum = 0;
	for (int i = 0; i < 3; ++i) {
		for (int k = i + 1; k < 3; ++k) {
			if (dir[i] != dir[k]) {
				++count[i];
				++count[k];
			}
		}

		if (count[i] == 2) {
			cnum = i;
			break;
		}
	}

	vec2f crossPos[2];
	int crossN = 0;
	for (int i = 0; i < 3; ++i) {
		if (i == cnum) continue;

		straightLine l = straightLine(triangle.point[cnum], triangle.point[i]);
		crossPos[crossN] = shp::GetCrossPoint(l, cutline);
		crossN++;
	}

	if (remaindir.x == dir[cnum].x || remaindir.y == dir[cnum].y) {
		triangle3v t = triangle3v(triangle.point[cnum], crossPos[0], crossPos[1]);
		returnTri.push_back(t);
	}
	else {
		int cnnum[2];
		int cc = 0;
		for (int i = 0; i < 3; ++i) {
			if (i == cnum) continue;
			cnnum[cc] = i;
			cc++;
		}
		triangle3v t1 = triangle3v(crossPos[1], crossPos[0], triangle.point[cnnum[0]]);
		returnTri.push_back(t1);

		if (shp::get_distance(crossPos[0], triangle.point[cnnum[1]]) >
			shp::get_distance(crossPos[1], triangle.point[cnnum[1]])) {
			triangle3v t2 = triangle3v(triangle.point[cnnum[0]], crossPos[1], triangle.point[cnnum[1]]);
			returnTri.push_back(t2);
		}
		else {
			triangle3v t2 = triangle3v(triangle.point[cnnum[0]], crossPos[0], triangle.point[cnnum[1]]);
			returnTri.push_back(t2);
		}

	}
	return returnTri;
}

vector<shp::triangle3v> shp::TrianglesCutStraightLine(vector<triangle3v> triangles, straightLine cutline, vec2f remaindir)
{
	vector<triangle3v> returnpoly;
	for (int i = 0; i < triangles.size(); ++i) {
		vec2f dir = shp::GetTriangleRangeInStraight(triangles[i], cutline);
		if (dir == remaindir) {
			returnpoly.push_back(triangles[i]);
		}
		else if (dir == vec2f(0, 0)) {
			vector<triangle3v> t = shp::TriangleCutStraightLine(triangles[i], cutline, remaindir);
			for (int k = 0; k < t.size(); ++k) {
				returnpoly.push_back(t[k]);
			}
		}
	}

	return returnpoly;
}

vector<shp::triangle3v> shp::FreePolygonInRectRange(vector<vec2f> freepolygon, rect4f rt)
{
	rect4f location = rect4f(freepolygon[0].x, freepolygon[0].y, freepolygon[0].x, freepolygon[0].y);
	for (int i = 0; i < freepolygon.size(); ++i) {
		vec2f pos = freepolygon[i];
		if (location.fx > pos.x) {
			location.fx = pos.x;
		}
		if (location.fy > pos.y) {
			location.fy = pos.y;
		}
		if (location.lx < pos.x) {
			location.lx = pos.x;
		}
		if (location.ly < pos.y) {
			location.ly = pos.y;
		}
	}

	vector<triangle3v> tris = shp::FreePolygonToTriangles(freepolygon);
	if (shp::bRectInRectRange(location, rt, true, false)) {
		return tris;
	}
	else {
		vec2f edge[4] = { vec2f(location.fx, location.fy),
		vec2f(location.fx, location.ly), vec2f(location.lx, location.ly),
		vec2f(location.lx, location.fy) };
		straightLine rtLine[4] = { straightLine(edge[0], edge[1]),
		straightLine(edge[1], edge[2]), straightLine(edge[2], edge[3]),
		straightLine(edge[3], edge[1]) };
		vector<triangle3v> returntri;
		for (int i = 0; i < 4; ++i) {
			vec2f rdir = vec2f(0, 0);
			switch (i) {
			case 0:
				rdir = vec2f(1, 0);
				break;
			case 1:
				rdir = vec2f(0, -1);
				break;
			case 2:
				rdir = vec2f(-1, 0);
				break;
			case 3:
				rdir = vec2f(0, 1);
				break;
			}
			returntri = shp::TrianglesCutStraightLine(tris, rtLine[i], rdir);
			tris.clear();
			tris = returntri;
		}

		return returntri;
	}
}

float shp::GetContactLocation(shp::vec2f pv1, shp::vec2f pv2) {
	if (pv1.y == 0 && pv2.y == 0) return pv1.x;
	return (fabsf(pv2.y) * pv1.x + fabsf(pv1.y) * pv2.x) / (fabsf(pv1.y) + fabsf(pv2.y));
}


bool shp::CollidCondition(vec2f rangeA, vec2f rangeB)
{
	if ((inside(rangeA.x, rangeB.x, rangeA.y) || inside(rangeA.x, rangeB.y, rangeA.y))
		|| (inside(rangeB.x, rangeA.x, rangeB.y) || inside(rangeB.x, rangeA.y, rangeB.y))) {
		return true;
	}
	return false;
}

bool shp::isCubeContactCube(cube6f c1, cube6f c2) {
	if ((CollidCondition(vec2f(c1.fx, c1.lx), vec2f(c2.fx, c2.lx))
		&& CollidCondition(vec2f(c1.fy, c1.ly), vec2f(c2.fy, c2.ly)))
		&& CollidCondition(vec2f(c1.fz, c1.lz), vec2f(c2.fz, c2.lz))) {
		return true;
	}
	return false;
}