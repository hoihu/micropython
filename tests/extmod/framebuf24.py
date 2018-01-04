try:
    import framebuf
except ImportError:
    print("SKIP")
    raise SystemExit

def printbuf():
    print("----+",end='')
    [print("----{:3}---+".format(i),end='') for i in range(w)]
    print()
    for y in range(h):
        print("{:3} | ".format(y),end='')
        for x in range(w):
            index = y*w*3 + x*3
            print("0x{:02X}{:02X}{:02X} | ".format(buf[index],buf[index+1],buf[index+2]),end='')
        print()
    print("----+"+"----------+"*w)

w = 10
h = 5
buf = bytearray(w * h * 3)
fbuf = framebuf.FrameBuffer(buf, w, h, framebuf.RGB888)

# fill
fbuf.fill(0xffff)
printbuf()
fbuf.fill(0x0000)
printbuf()

# put pixel
fbuf.pixel(0, 0, 0xaabbcc)
fbuf.pixel(3, 0, 0xaabbcc)
fbuf.pixel(0, 4, 0xddeeff)
fbuf.pixel(3, 4, 0xddeeff)
printbuf()

# get pixel
print(fbuf.pixel(0, 4), fbuf.pixel(1, 1))

# scroll
fbuf.fill(0x000000)
fbuf.pixel(2, 2, 0xffffff)
printbuf()
fbuf.scroll(0, 1)
printbuf()
fbuf.scroll(1, 0)
printbuf()
fbuf.scroll(-1, -2)
printbuf()

w2 = 2
h2 = 3
buf2 = bytearray(w2 * h2 * 3)
fbuf2 = framebuf.FrameBuffer(buf2, w2, h2, framebuf.RGB888)

fbuf2.fill(0x000000)
fbuf2.pixel(0, 0, 0x0aabb0)
fbuf2.pixel(0, 2, 0xaa0000)
fbuf2.pixel(1, 0, 0x00bb00)
fbuf2.pixel(1, 2, 0xa0000e)
fbuf.fill(0xffff)
fbuf.blit(fbuf2, 3, 3, 0x000000)
fbuf.blit(fbuf2, -1, -1, 0x000000)
fbuf.blit(fbuf2, 16, 16, 0x000000)
printbuf()
