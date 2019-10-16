import sys


c_aabbWidth = 2
c_aabbDistance = 2

aabbGrid = [3, 1, 3]

def getBasePosition():
    result = (\
    -(aabbGrid[0] * c_aabbWidth + (aabbGrid[0] - 1) * c_aabbDistance) / 2.0,
    -(aabbGrid[1] * c_aabbWidth + (aabbGrid[1] - 1) * c_aabbDistance) / 2.0,
    -(aabbGrid[2] * c_aabbWidth + (aabbGrid[2] - 1) * c_aabbDistance) / 2.0)
    return result

def centerForCoord(xcoord, ycoord, zcoord, basePos):
    cx = basePos[0] + xcoord * (c_aabbWidth + c_aabbDistance) + c_aabbWidth / 2.0
    cy = basePos[1] + ycoord * (c_aabbWidth + c_aabbDistance) + c_aabbWidth / 2.0
    cz = basePos[2] + zcoord * (c_aabbWidth + c_aabbDistance) + c_aabbWidth / 2.0

    return (cx, cy, cz)

def getCenters():
    result = []
    basePos = getBasePosition()
    for i in range(aabbGrid[0]):
        for j in range(aabbGrid[1]):
            for k in range(aabbGrid[2]):
                result.append(centerForCoord(i, j, k, basePos))


    return result



def main():
    print("Base position:")
    print(getBasePosition())
    print("Centers:")
    print(getCenters())

if __name__ == "__main__":
    main()
