import cv2
from sys import argv
import os

path = ''


def main():
    if len(argv) > 1:
        path = argv[1]

    base_dir = os.getcwd()
    directory = os.path.join(base_dir, path)

    if not os.path.exists(directory):
        print "Invalid Path - Directory does not exist"
        exit()

    img_dirs = [
        image for image in os.listdir(directory) if os.path.splitext(image)[1][1:] in ['png', 'jpg', 'bmp']
    ]

    out = cv2.VideoWriter(os.path.basename(os.path.normpath(directory)) + '.avi',
                          cv2.cv.FOURCC('X', 'V', 'I', 'D'), 20.0, (640, 480))

    for image in sorted(img_dirs):
        frame = cv2.imread(os.path.join(directory, image))
        out.write(frame)

    out.release()


if __name__ == '__main__':
    main()
