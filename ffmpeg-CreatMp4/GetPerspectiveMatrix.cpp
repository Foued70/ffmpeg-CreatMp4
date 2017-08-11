//
// Created by yfei on 17-8-11.
//

#include "GetPerspectiveMatrix.h"

Matrix<double ,3,3> GetPerspectiveMatrix::PerspectiveMatrix() {

    Matr(0, 0) = x[0];
    Matr(0, 1) = y[0];
    Matr(0, 2) = 1;
    Matr(0, 3) = 0;
    Matr(0, 4) = 0;
    Matr(0, 5) = 0;
    Matr(0, 6) = -1 * x[0] * u[0];
    Matr(0, 7) = -1 * y[0] * u[0];
    Matr(1, 0) = 0;
    Matr(1, 1) = 0;
    Matr(1, 2) = 0;
    Matr(1, 3) = x[0];
    Matr(1, 4) = y[0];
    Matr(1, 5) = 1;
    Matr(1, 6) = -1 * x[0] * v[0];
    Matr(1, 7) = -1 * y[0] * v[0];

    Matr(2, 0) = x[1];
    Matr(2, 1) = y[1];
    Matr(2, 2) = 1;
    Matr(2, 3) = 0;
    Matr(2, 4) = 0;
    Matr(2, 5) = 0;
    Matr(2, 6) = -1 * x[1] * u[1];
    Matr(2, 7) = -1 * y[1] * u[1];
    Matr(3, 0) = 0;
    Matr(3, 1) = 0;
    Matr(3, 2) = 0;
    Matr(3, 3) = x[1];
    Matr(3, 4) = y[1];
    Matr(3, 5) = 1;
    Matr(3, 6) = -1 * x[1] * v[1];
    Matr(3, 7) = -1 * y[1] * v[1];

    Matr(4, 0) = x[2];
    Matr(4, 1) = y[2];
    Matr(4, 2) = 1;
    Matr(4, 3) = 0;
    Matr(4, 4) = 0;
    Matr(4, 5) = 0;
    Matr(4, 6) = -1 * x[2] * u[2];
    Matr(4, 7) = -1 * y[2] * u[2];
    Matr(5, 0) = 0;
    Matr(5, 1) = 0;
    Matr(5, 2) = 0;
    Matr(5, 3) = x[2];
    Matr(5, 4) = y[2];
    Matr(5, 5) = 1;
    Matr(5, 6) = -1 * x[2] * v[2];
    Matr(5, 7) = -1 * y[2] * v[2];

    Matr(6, 0) = x[3];
    Matr(6, 1) = y[3];
    Matr(6, 2) = 1;
    Matr(6, 3) = 0;
    Matr(6, 4) = 0;
    Matr(6, 5) = 0;
    Matr(6, 6) = -1 * x[3] * u[3];
    Matr(6, 7) = -1 * y[3] * u[3];
    Matr(7, 0) = 0;
    Matr(7, 1) = 0;
    Matr(7, 2) = 0;
    Matr(7, 3) = x[3];
    Matr(7, 4) = y[3];
    Matr(7, 5) = 1;
    Matr(7, 6) = -1 * x[3] * v[3];
    Matr(7, 7) = -1 * y[3] * v[3];

    auto r = Matr.inverse();
    Matrix<double, 8, 1> image_point;
    image_point << u[0], v[0], u[1], v[1], u[2], v[2], u[3], v[3];

    auto rest = Matr.inverse() * image_point;

    Juzhen << rest(0), rest(1), rest(2), rest(3), rest(4), rest(5), rest(6), rest(7), 1;

    return Juzhen;
}