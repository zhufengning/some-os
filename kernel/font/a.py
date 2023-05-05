from bdfparser import Font
font = Font("zpix.bdf")
c = font.drawall()
f = open("t.c", "w")
f.write("const char FONT[] = " + str(c.todata(3)).replace("[","{").replace("]","}")+";")
s = "\nconst int FONT_CODE[] = {"
for g in font.iterglyphs():
    s += str(ord(g.chr()))
    s += ","

s += "};"
f.write(s)