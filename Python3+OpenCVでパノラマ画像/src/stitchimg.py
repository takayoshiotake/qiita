import argparse
import cv2

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Stitch given images.')
    parser.add_argument("inputs", metavar='input', type=str, nargs='+', help='input file')
    parser.add_argument("--output", type=str, default='a.png', help='output file (default=a.png)')
    args = parser.parse_args()

    input_images = []
    for i in args.inputs:
        image = cv2.imread(i)
        if image is None:
            print(f'Error: Unable to open file "{i}".')
            exit()
        input_images.append(image)

    if len(input_images) == 1:
        cv2.imwrite(args.output, input_images[0])
    else:
        stitcher = cv2.createStitcher(True)
        stitched = stitcher.stitch(input_images)
        cv2.imwrite(args.output, stitched[1])
