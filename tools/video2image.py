#!/bin/python3

import cv2;

def Convert2BitMAP(frame,destPath):
    ret,img2=cv2.threshold(frame,10,255,cv2.THRESH_BINARY)
    img3=cv2.resize(img2,(128,64),cv2.INTER_AREA)
    ret,img3=cv2.threshold(img3,10,255,cv2.THRESH_BINARY)
    # cv2.imshow("a",img3)
    # cv2.waitKey(0)
    cv2.imwrite("/home/shaozi/视频/apple/"+str(c).zfill(8)+'.bmp',img3)


def Video2BitMAP(srcFile,destFolder):
    vc=cv2.VideoCapture(srcFile)
    c=0
    rval =vc.isOpened()

    while rval:
        c=c+1
        rval,frame=vc.read()
        
        if rval:
            Convert2BitMAP(frame,destFolder+str(c).zfill(8)+'.bmp')
        else:
            break
    vc.release()

Video2BitMAP('/home/shaozi/视频/bad_apple.mp4',"/home/shaozi/视频/apple/")