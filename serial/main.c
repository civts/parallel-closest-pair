#include <stdio.h>
#include <stdlib.h>

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

int main() {
    int i;
    PointVec point_vec;

    point_vec = load_data("data/test_dataset.txt");
    
    for (i = 0; i<point_vec.length; i++) {
        printf("%d %d\n", point_vec.points[i].x, point_vec.points[i].y);
    }

    return 0;
}