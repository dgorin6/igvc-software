import sys

sys.path.append('/usr/local/lib/python2.7/site-packages')

import cv2
import networkx as nx
import numpy as np
import pylab as pl
import random as rand
from math import sqrt, atan, degrees

ground_plane_size = 90

image_size = 4000

def genCourse():
    blank_image = np.zeros((image_size,image_size,3))
    b_channel, g_channel, r_channel = cv2.split(blank_image)
    alpha_channel = np.ones(b_channel.shape, dtype=b_channel.dtype) * 255

    str = sys.stdin.readline()
    if str == 'basic\n':
        width = rand.randint(3,7)
        pixWidth = convert_distance_to_pixel(width)
        length = rand.randint(20,86)
        pixLength = convert_distance_to_pixel(length)
        create_line(2000, 4000 - int(pixLength/2), length, width, alpha_channel)

    if str == 'intermediate\n':
        outerRad = rand.randint(10, 15);
        innerRad = outerRad - 2;
        centerRad = innerRad - 1;

        create_circle(convert_distance_to_pixel(39.2), image_size / 2, innerRad, outerRad, 225, 220, alpha_channel, 0)
        create_circle(convert_distance_to_pixel(39.2), image_size / 2, 0, centerRad, 0, 360, alpha_channel, 0)
        create_circle(convert_distance_to_pixel(39.2), image_size / 2, centerRad, innerRad, 250, 290, alpha_channel, 0)

    img_RGBA = cv2.merge((b_channel, g_channel, r_channel, alpha_channel))
    cv2.imwrite("blended_texture.png", img_RGBA)





def create_circle(centerX, centerY, radiusIn, radiusOut, orientationStart, orientationEnd, channel, brightness):
    for i in range(centerX - convert_distance_to_pixel(radiusOut), centerX + convert_distance_to_pixel(radiusOut)):
        for j in range(centerY - convert_distance_to_pixel(radiusOut), centerY + convert_distance_to_pixel(radiusOut)):
            distance = convert_pixel_to_distance(centerX, centerY, i, j)
            # gets angle from our defined zero (right)
            theta = -1
            if i - centerX != 0:
                theta = degrees(atan((float(j) - centerY)/(i - centerX)))

            if i >= centerX and j <= centerY:
                theta += 360
            elif i <= centerX and j >= centerY:
                theta += 180
            elif i <= centerX and j <= centerY:
                theta += 180

            if distance <= radiusOut and distance >= radiusIn:
                if theta >= orientationStart and theta <= orientationEnd:
                    channel[j, i] = brightness
                elif orientationStart > orientationEnd:
                    if theta > orientationStart or theta < orientationEnd:
                        channel[j, i] = brightness

    return channel

def convert_pixel_to_distance(pixel1X, pixel1Y, pixel2X, pixel2Y):
    xDist = (pixel1X - pixel2X) * (float(ground_plane_size) / image_size)
    yDist = (pixel1Y - pixel2Y) * (float(ground_plane_size) / image_size)
    return sqrt(xDist**2 + yDist**2)

def convert_distance_to_pixel(distance):
    return int(round(distance * (float(image_size) / ground_plane_size)))

def create_line(startX, startY, width, length, alpha_channel):
    half_width = float(width) / 2
    for i in range(startY - convert_distance_to_pixel(half_width), startY + convert_distance_to_pixel(half_width)):
        for j in range(startX, startX + convert_distance_to_pixel(length)):
            alpha_channel[i, j] = 0
    return alpha_channel


def main():
    genCourse()
if __name__=='__main__':
    main()
