#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int x, y;
} Point;

typedef struct {
    int length;
    Point *points;
} PointVec;

PointVec load_data(char *path) {
    unsigned int n;
    FILE *f = NULL;
    int i;
    Point tmp;
    PointVec point_vec;

    // Open file
    f = fopen(path, "r");
    if (f == NULL)
    {
        printf("Can't open file %s", path);
        exit(1);
    }

    // Read number of points
    fscanf(f, "%d", &n);

    // Setup vector of points
    point_vec.length = n;
    point_vec.points = malloc(n*sizeof(Point));

    // Read points
    for (i = 0; i<n; i++) { 
        fscanf(f, "%d %d", &(tmp.x), &(tmp.y));

        point_vec.points[i] = tmp;
    }
    
    return point_vec;
}

// Compare functions used to sort the points
int compare_x(const void *e1, const void *e2) {
    Point *p1 = (Point *)e1;
    Point *p2 = (Point *)e2;
    return (p1->x - p2->x);
}

int compare_y(const void *e1, const void *e2) {
    Point *p1 = (Point *)e1;
    Point *p2 = (Point *)e2;
    return (p1->y - p2->y);
}

// Compute distance between 2 points
float distance(Point p1, Point p2) {
    return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
}

float closestPoints(Point* points, int n) {
    // Base case here

    int mid = n/2;

    // Split array
    // Find left and right minimum distance
    // Check points in the middle
}

int main() {
    int i;
    PointVec point_vec;

    point_vec = load_data("../data/test_dataset.txt");
    
    // Sort array based on x coordinate
    qsort(point_vec.points, point_vec.length, sizeof(Point), compare_x);

    for (i = 0; i<point_vec.length; i++) {
        printf("%d %d\n", point_vec.points[i].x, point_vec.points[i].y);
    }

    return 0;
}