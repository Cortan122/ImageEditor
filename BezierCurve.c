/*
An Algorithm for Automatically Fitting Digitized Curves
by Philip J. Schneider
from "Graphics Gems", Academic Press, 1990
*/

#include "BezierCurve.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* return the distance between two points */
static float V2DistanceBetween2Points(Vector2d* a, Vector2d* b) {
  float dx = a->x - b->x;
  float dy = a->y - b->y;
  return sqrt(dx * dx + dy * dy);
}

/* returns squared length of input vector */
static float V2SquaredLength(Vector2d* a) {
  return a->x * a->x + a->y * a->y;
}

/* returns length of input vector */
static float V2Length(Vector2d* a) {
  return sqrt(V2SquaredLength(a));
}

static Vector2d* V2Scale(Vector2d* v, float newlen) {
  float len = V2Length(v);
  if (len != 0.0) {
    v->x *= newlen / len;
    v->y *= newlen / len;
  }
  return v;
}

/* negates the input vector and returns it */
static Vector2d* V2Negate(Vector2d* v) {
  v->x = -v->x;
  v->y = -v->y;
  return v;
}

/* return the dot product of vectors a and b */
static float V2Dot(Vector2d* a, Vector2d* b) {
  return a->x * b->x + a->y * b->y;
}

/* normalizes the input vector and returns it */
static Vector2d* V2Normalize(Vector2d* v) {
  float len = V2Length(v);
  if (len != 0.0) {
    v->x /= len;
    v->y /= len;
  }
  return v;
}

/* return vector sum res = a+b */
static Vector2d* V2Add(Vector2d* a, Vector2d* b, Vector2d* res) {
  res->x = a->x + b->x;
  res->y = a->y + b->y;
  return res;
}

static Vector2d V2AddII(Vector2d a, Vector2d b) {
  Vector2d c;
  c.x = a.x + b.x;
  c.y = a.y + b.y;
  return c;
}

static Vector2d V2ScaleIII(Vector2d v, float s) {
  Vector2d result;
  result.x = v.x * s;
  result.y = v.y * s;
  return result;
}

static Vector2d V2SubII(Vector2d a, Vector2d b) {
  Vector2d c;
  c.x = a.x - b.x;
  c.y = a.y - b.y;
  return c;
}

/**
 * Evaluate a Bezier curve at a particular parameter value
 *
 * @param degree The degree of the bezier curve
 * @param V      Array of control points
 * @param t      Parametric value to find point for
 */
Vector2d Bezier(int degree, Vector2d* V, float t) {
  int i, j;
  Vector2d Q;      /* Point on curve at parameter t */
  Vector2d* Vtemp; /* Local copy of control points */

  /* Copy array */
  Vtemp = malloc((degree + 1) * sizeof(Vector2d));
  for (i = 0; i <= degree; i++) {
    Vtemp[i] = V[i];
  }

  /* Triangle computation */
  for (i = 1; i <= degree; i++) {
    for (j = 0; j <= degree - i; j++) {
      Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j + 1].x;
      Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j + 1].y;
    }
  }

  Q = Vtemp[0];
  free(Vtemp);
  return Q;
}

/**
 * Use Newton-Raphson iteration to find better root.
 *
 * @param Q Current fitted curve
 * @param P Digitized point
 * @param u Parameter value for "P"
 */
static float NewtonRaphsonRootFind(Vector2d* Q, Vector2d P, float u) {
  float numerator, denominator;
  Vector2d Q1[3], Q2[2];    /* Q' and Q'' */
  Vector2d Q_u, Q1_u, Q2_u; /*u evaluated at Q, Q', & Q'' */
  float uPrime;             /* Improved u */
  int i;

  /* Compute Q(u) */
  Q_u = Bezier(3, Q, u);

  /* Generate control vertices for Q' */
  for (i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i + 1].x - Q[i].x) * 3.0;
    Q1[i].y = (Q[i + 1].y - Q[i].y) * 3.0;
  }

  /* Generate control vertices for Q'' */
  for (i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i + 1].x - Q1[i].x) * 2.0;
    Q2[i].y = (Q1[i + 1].y - Q1[i].y) * 2.0;
  }

  /* Compute Q'(u) and Q''(u) */
  Q1_u = Bezier(2, Q1, u);
  Q2_u = Bezier(1, Q2, u);

  /* Compute f(u)/f'(u) */
  numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
  denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) + (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);

  /* u = u - f(u)/f'(u) */
  uPrime = u - (numerator / denominator);
  return uPrime;
}

/**
 * Given set of points and their parameterization, try to find a better
 * parameterization.
 *
 * @param d        Array of digitized points
 * @param first    Indices defining region
 * @param last     Indices defining region
 * @param u        Current parameter values
 * @param bezCurve Current fitted curve
 */
static float* Reparameterize(Vector2d* d, int first, int last, float* u, Vector2d* bezCurve) {
  int nPts = last - first + 1;
  int i;
  float* uPrime; /* New parameter values */

  uPrime = malloc(nPts * sizeof(float));
  for (i = first; i <= last; i++) {
    uPrime[i - first] = NewtonRaphsonRootFind(bezCurve, d[i], u[i - first]);
  }
  return uPrime;
}

/*
 * Bezier multipliers
 */
static float B0(float u) {
  float tmp = 1.0 - u;
  return tmp * tmp * tmp;
}

static float B1(float u) {
  float tmp = 1.0 - u;
  return 3 * u * (tmp * tmp);
}

static float B2(float u) {
  float tmp = 1.0 - u;
  return 3 * u * u * tmp;
}

static float B3(float u) {
  return u * u * u;
}

/**
 * Approximate unit tangent at left endpoint of digitized curve
 *
 * @param d   Digitized points
 * @param end Index to "left" end of region
 */
static Vector2d ComputeLeftTangent(Vector2d* d, int end) {
  Vector2d tHat1;
  tHat1 = V2SubII(d[end + 1], d[end]);
  V2Normalize(&tHat1);
  return tHat1;
}

/**
 * Approximate unit tangent at right endpoint of digitized curve
 *
 * @param d   Digitized points
 * @param end Index to "right" end of region
 */
static Vector2d ComputeRightTangent(Vector2d* d, int end) {
  Vector2d tHat2;
  tHat2 = V2SubII(d[end - 1], d[end]);
  V2Normalize(&tHat2);
  return tHat2;
}

/**
 * Approximate unit tangent at the "center" of digitized curve
 *
 * @param d      Digitized points
 * @param center Index to point inside region
 */
static Vector2d ComputeCenterTangent(Vector2d* d, int center) {
  Vector2d V1, V2, tHatCenter;

  V1 = V2SubII(d[center - 1], d[center]);
  V2 = V2SubII(d[center], d[center + 1]);
  tHatCenter.x = (V1.x + V2.x) / 2.0;
  tHatCenter.y = (V1.y + V2.y) / 2.0;
  V2Normalize(&tHatCenter);
  return tHatCenter;
}

/**
 * Assign parameter values to digitized points
 * using relative distances between points.
 *
 * @param d     Array of digitized points
 * @param first Indices defining region
 * @param last  Indices defining region
 */
static float* ChordLengthParameterize(Vector2d* d, int first, int last) {
  int i;
  float* u; /* Parameterization */

  u = malloc((last - first + 1) * sizeof(float));

  u[0] = 0.0;
  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first - 1] + V2DistanceBetween2Points(&d[i], &d[i - 1]);
  }

  for (i = first + 1; i <= last; i++) {
    u[i - first] = u[i - first] / u[last - first];
  }

  return u;
}

/**
 * Find the maximum squared distance of digitized points
 * to fitted curve.
 *
 * @param d          Array of digitized points
 * @param first      Indices defining region
 * @param last       Indices defining region
 * @param bezCurve   Fitted Bezier curve
 * @param u          Parameterization of points
 * @param splitPoint Point of maximum error
 */
static float ComputeMaxError(Vector2d* d, int first, int last, Vector2d* bezCurve, float* u, int* splitPoint) {
  int i;
  float maxDist; /* Maximum error */
  float dist;    /* Current error */
  Vector2d P;    /* Point on curve */
  Vector2d v;    /* Vector from point to curve */

  *splitPoint = (last - first + 1) / 2;
  maxDist = 0.0;
  for (i = first + 1; i < last; i++) {
    P = Bezier(3, bezCurve, u[i - first]);
    v = V2SubII(P, d[i]);
    dist = V2SquaredLength(&v);
    if (dist >= maxDist) {
      maxDist = dist;
      *splitPoint = i;
    }
  }
  return maxDist;
}

/**
 * Use least-squares method to find Bezier control points for region.
 *
 * @param d      Array of digitized points
 * @param first  Indices defining region
 * @param last   Indices defining region
 * @param uPrime Parameter values for region
 * @param tHat1  Unit tangents at endpoints
 * @param tHat2  Unit tangents at endpoints
 */
static Vector2d* GenerateBezier(Vector2d* d, int first, int last, float* uPrime, Vector2d tHat1, Vector2d tHat2) {
  int i;
  int nPts;                            /* Number of pts in sub-curve */
  float C[2][2];                       /* Matrix C */
  float X[2];                          /* Matrix X */
  float det_C0_C1, det_C0_X, det_X_C1; /* Determinants of matrices */
  float alpha_l, alpha_r;              /* Alpha values, left and right */
  Vector2d tmp;                        /* Utility variable */
  Vector2d* bezCurve;                  /* RETURN bezier curve ctl pts */

  bezCurve = malloc(4 * sizeof(Vector2d));
  nPts = last - first + 1;

  /* Create the C and X matrices */
  C[0][0] = 0.0;
  C[0][1] = 0.0;
  C[1][0] = 0.0;
  C[1][1] = 0.0;
  X[0] = 0.0;
  X[1] = 0.0;

  for (i = 0; i < nPts; i++) {
    Vector2d v1 = tHat1;
    Vector2d v2 = tHat2;
    V2Scale(&v1, B1(uPrime[i]));
    V2Scale(&v2, B2(uPrime[i]));

    C[0][0] += V2Dot(&v1, &v1);
    C[0][1] += V2Dot(&v1, &v2);
    C[1][0] = C[0][1];
    C[1][1] += V2Dot(&v2, &v2);

    tmp =
        V2SubII(d[first + i],
                V2AddII(V2ScaleIII(d[first], B0(uPrime[i])),
                        V2AddII(V2ScaleIII(d[first], B1(uPrime[i])),
                                V2AddII(V2ScaleIII(d[last], B2(uPrime[i])), V2ScaleIII(d[last], B3(uPrime[i]))))));

    X[0] += V2Dot(&v1, &tmp);
    X[1] += V2Dot(&v2, &tmp);
  }

  /* Compute the determinants of C and X */
  det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
  det_C0_X = C[0][0] * X[1] - C[0][1] * X[0];
  det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

  /* Finally, derive alpha values */
  if (det_C0_C1 == 0.0) {
    det_C0_C1 = (C[0][0] * C[1][1]) * 10e-12;
  }
  alpha_l = det_X_C1 / det_C0_C1;
  alpha_r = det_C0_X / det_C0_C1;

  /* If alpha negative, use the Wu/Barsky heuristic (see text) */
  /* (if alpha is 0, you get coincident control points that lead to
   * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
  if (alpha_l < 1.0e-6 || alpha_r < 1.0e-6) {
    float dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;

    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
    V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
    return bezCurve;
  }

  /* First and last control points of the Bezier curve are */
  /* positioned exactly at the first and last data points */
  /* Control points 1 and 2 are positioned an alpha distance out */
  /* on the tangent vectors, left and right, respectively */
  bezCurve[0] = d[first];
  bezCurve[3] = d[last];
  V2Add(&bezCurve[0], V2Scale(&tHat1, alpha_l), &bezCurve[1]);
  V2Add(&bezCurve[3], V2Scale(&tHat2, alpha_r), &bezCurve[2]);
  return bezCurve;
}

/**
 * Fit a Bezier curve to a (sub)set of digitized points
 *
 * @param d     Array of digitized points
 * @param first Indices of first and last pts in region
 * @param last  Indices of first and last pts in region
 * @param tHat1 Unit tangent vectors at endpoints
 * @param tHat2 Unit tangent vectors at endpoints
 * @param error User-defined error squared
 */
static void FitCubic(Vector2d* d, int first, int last, Vector2d tHat1, Vector2d tHat2, float error,
                     BezierPath* res) {
  Vector2d* bezCurve;    /* Control points of fitted Bezier curve*/
  float* u;              /* Parameter values for point */
  float* uPrime;         /* Improved parameter values */
  float maxError;        /* Maximum fitting error */
  int splitPoint;        /* Point to split point set at */
  int nPts;              /* Number of points in subset */
  float iterationError;  /* Error below which you try iterating */
  int maxIterations = 4; /* Max times to try iterating */
  Vector2d tHatCenter;   /* Unit tangent vector at splitPoint */
  int i;

  iterationError = error * error;
  nPts = last - first + 1;

  /* Use heuristic if region only has two points in it */
  if (nPts == 2) {
    float dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;

    bezCurve = malloc(4 * sizeof(Vector2d));
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
    V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
    memcpy(sb_add(*res, 1), bezCurve, sizeof(Vector2d) * 4);
    free(bezCurve);
    return;
  }

  /* Parameterize points, and attempt to fit curve */
  u = ChordLengthParameterize(d, first, last);
  bezCurve = GenerateBezier(d, first, last, u, tHat1, tHat2);

  /* Find max deviation of points to fitted curve */
  maxError = ComputeMaxError(d, first, last, bezCurve, u, &splitPoint);
  if (maxError < error) {
    memcpy(sb_add(*res, 1), bezCurve, sizeof(Vector2d) * 4);
    free(u);
    free(bezCurve);
    return;
  }

  /* If error not too large, try some reparameterization */
  /* and iteration */
  if (maxError < iterationError) {
    for (i = 0; i < maxIterations; i++) {
      uPrime = Reparameterize(d, first, last, u, bezCurve);
      free(bezCurve);
      bezCurve = GenerateBezier(d, first, last, uPrime, tHat1, tHat2);
      maxError = ComputeMaxError(d, first, last, bezCurve, uPrime, &splitPoint);
      if (maxError < error) {
        memcpy(sb_add(*res, 1), bezCurve, sizeof(Vector2d) * 4);
        free(u);
        free(uPrime);
        free(bezCurve);
        return;
      }
      free(u);
      u = uPrime;
    }
  }

  /* Fitting failed -- split at max error point and fit recursively */
  free(u);
  free(bezCurve);
  tHatCenter = ComputeCenterTangent(d, splitPoint);
  FitCubic(d, first, splitPoint, tHat1, tHatCenter, error, res);
  V2Negate(&tHatCenter);
  FitCubic(d, splitPoint, last, tHatCenter, tHat2, error, res);
}

/**
 * Fit a Bezier curve to a set of digitized points
 *
 * @param d     Array of digitized points
 * @param nPts  Number of digitized points
 * @param error User-defined error squared
 * @returns stretchy_buffer of Bezier curves
 */
BezierPath FitCurve(Vector2d* d, int nPts, float error) {
  if (nPts < 2) return NULL;
  BezierPath res = NULL;
  Vector2d tHat1 = ComputeLeftTangent(d, 0);
  Vector2d tHat2 = ComputeRightTangent(d, nPts - 1);
  FitCubic(d, 0, nPts - 1, tHat1, tHat2, error, &res);
  return res;
}
