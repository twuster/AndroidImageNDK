from scipy import misc
import numpy as np

def getcontour(image):
  height = image.shape[0]
  width = image.shape[1]

  points = []
  points_dict = {}
  explored = {}

  for y in xrange(height):
    for x in xrange(width):
      if image[y, x] > 128:
        points.append((y, x))
        points_dict[(y, x)] = 0

  check_index = 0
  boxes = []
  while len(explored) < len(points):
    # Select a point
    point = points[check_index]

    if point in explored:
      check_index += 1
      continue

    # Get a bounding box
    top = point[0]
    bottom = point[0]
    left = point[1]
    right = point[1]
    
    fringe = [point]
    explored[point] = 0
    print "bfs " + str(check_index)
    while len(fringe) > 0:
      #print fringe
      point = fringe.pop()

      if point[0] < top:
        top = point[0]
      elif point[0] > bottom:
        bottom = point[0]
      if point[1] < left:
        left = point[1]
      elif point[1] > right:
        right = point[1]

      for y in xrange(-1, 2):
        for x in xrange(-1, 2):
          check_pt = (point[0]+y, point[1]+x)
          if check_pt not in explored and check_pt in points:
            fringe.append(check_pt)
            explored[check_pt] = 0

    boxes.append((top, right, bottom, left))
    check_index += 1
    print check_index

  return boxes

if __name__ == '__main__':
  img = misc.imread('./test.png', True)
  boxes = getcontour(img)
  i = 0
  for box in boxes:
    misc.imsave("box" + str(i) + ".png", img[box[0]:box[2], box[3]:box[1]])
    i += 1
