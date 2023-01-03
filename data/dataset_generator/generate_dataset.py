import random
import argparse
import math
import time


def generate_points(N, minX, maxX, minY, maxY):
    if (maxX - minX) * (maxY - minY) < N:
        minX = -2*N
        maxX = 2*N
        minY = -2*N
        maxY = 2*N
        print(
            f"""Impossible to generate {N} points with the given X and Y constraints. 
              Increasing automatically the constraints to X: [{minX}, {maxX}] Y: [{minY}, {maxY}]""")

    generated = 0
    eps = math.sqrt(N)
    points_dict = {}

    while generated < N:
        gen_x = True
        gen_y = True

        while gen_x:
            x = random.randint(minX, maxX)
            y = random.randint(minY, maxY)
            if x in points_dict:
                if y in points_dict[x]:
                    # if there is still space along Y generate new y with that x, otherwise change x
                    if len(points_dict[x]) <= (maxY - minY) - eps:
                        while gen_y:
                            y = random.randint(minY, maxY)
                            if y not in points_dict[x]:
                                gen_x = False
                                gen_y = False
                                points_dict[x].append(y)
                                generated += 1
                    else:
                        gen_x = True
                # new point x y
                else:
                    gen_x = False
                    gen_y = False
                    points_dict[x].append(y)
                    generated += 1
            else:
                gen_x = False
                gen_y = False
                points_dict[x] = []
                points_dict[x].append(y)
                generated += 1

    return points_dict


def write_to_file(N, points, path):
    with open(path, 'w') as f:
        f.write(f"{N}\n")
        for x in points:
            for y in points[x]:
                f.write(f"{x} {y}\n")


if __name__ == '__main__':

    parser = argparse.ArgumentParser(prog="Dataset Generatore",
                                     description="Generates a set of N random 2D points")

    parser.add_argument('N', default=100, type=int,
                        help='Number of points to generate')

    parser.add_argument('--minX', default=-100, type=int, required=False,
                        help='Minimum X coordinate')

    parser.add_argument('--maxX', default=100, type=int, required=False,
                        help='Maximum X coordinate')

    parser.add_argument('--minY', default=-100, type=int, required=False,
                        help='Minimum Y coordinate')

    parser.add_argument('--maxY', default=100, type=int, required=False,
                        help='Maximum Y coordinate')

    args = parser.parse_args()

    N = args.N
    minX = args.minX
    maxX = args.maxX
    minY = args.minY
    maxY = args.maxY

    start_time = time.time()

    points = generate_points(N, minX, maxX, minY, maxY)

    write_to_file(N, points, f'../{N}.txt')

    print(f"Total time: {time.time() - start_time:.2f} s")
