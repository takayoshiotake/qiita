import argparse
import cv2
import numpy as np

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Overlay given images with perfect matching.')
    parser.add_argument("inputs", metavar='input', type=str, nargs=2,
                        help='input file (inputs are (left, right) or (top, bottom))')
    parser.add_argument("--output", type=str, default='a.png',
                        help='output file (default=a.png)')
    parser.add_argument("--direction", choices=['h', 'v'], default='h',
                        help='overlay direction (default=h, h=horizontal, v=vertical)')
    parser.add_argument("--threshold", type=float, default=0,
                        help='matching threshold (default=0)')
    args = parser.parse_args()

    input_images = []
    for i in args.inputs:
        image = cv2.imread(i)
        if image is None:
            print(f'Error: Unable to open file "{i}".')
            exit()
        input_images.append(image)

    if args.direction == 'h':
        if input_images[0].shape[0] != input_images[1].shape[0]:
            print('Error: height not matched')
            exit()
        if input_images[0].shape[2] != input_images[1].shape[2]:
            print('Error: channels not matched')
            exit()
        match_h = -1
        for h in range(input_images[1].shape[1]):
            # if np.array_equal(input_images[0][:, -1], input_images[1][:, h]):
            if np.allclose(input_images[0][:, -1], input_images[1][:, h], args.threshold):
                match_h = h + 1
                break
        if match_h == -1:
            print(f'Error: Unable to match.')
            exit()
        # print(f'match_h={match_h}')
        stitched = np.hstack((input_images[0], input_images[1][:, match_h:]))
        cv2.imwrite(args.output, stitched)
    else:
        if input_images[0].shape[1] != input_images[1].shape[1]:
            print('Error: width not matched')
            exit()
        if input_images[0].shape[2] != input_images[1].shape[2]:
            print('Error: channels not matched')
            exit()
        match_v = -1
        for v in range(input_images[1].shape[0]):
            # if np.array_equal(input_images[0][-1], input_images[1][v]):
            if np.allclose(input_images[0][-1], input_images[1][v], args.threshold):
                match_v = v + 1
                break
        if match_v == -1:
            print(f'Error: Unable to match.')
            exit()
        # print(f'match_v={match_v}')
        stitched = np.vstack((input_images[0], input_images[1][match_v:]))
        cv2.imwrite(args.output, stitched)
