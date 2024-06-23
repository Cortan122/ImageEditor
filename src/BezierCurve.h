#pragma once
#include "stretchy_buffer.h"

typedef struct Vector2d {
  float x;
  float y;
} Vector2d;

typedef Vector2d BezierCurve[4];

// stretchy_buffer, free with sb_free
typedef BezierCurve* BezierPath;

/**
 * Evaluate a Bezier curve at a particular parameter value
 *
 * @param degree The degree of the bezier curve
 * @param V      Array of control points
 * @param t      Parametric value to find point for
 */
Vector2d Bezier(int degree, Vector2d* V, float t);

/**
 * Fit a Bezier curve to a set of digitized points
 *
 * @param d     Array of digitized points
 * @param nPts  Number of digitized points
 * @param error User-defined error squared
 * @returns stretchy_buffer of Bezier curves
 */
BezierPath FitCurve(Vector2d* d, int nPts, float error);
