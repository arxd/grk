from PIL import Image
import sys

class FontFile():
	def __init__(self, filename, w, h):
		self.csize = (w,h)
		self.cpr = 128//w
		self.img = Image.open(filename)
		
	def get_ascii(self, ascii):
		ascii -= 32
		return self.img.crop( (
			(ascii%16)*self.csize[0], #left
			(ascii//16)*self.csize[1], #top
			((ascii%16)+1)*self.csize[0], #right
			((ascii//16)+1)*self.csize[1], #bottom
			))
	
	def get_bits(self):
		return list(map(lambda x: 1 if x else 0, self.img.getdata(0)))
	
	def get_ascii_binary(self, ascii):
		data = list(map(lambda x: 1 if x else 0, font.get_ascii(ascii).getdata(0)))
		bytes = []
		print("-=%3d=-"%ascii)
		for row in range(self.csize[1]):
			row_data = data[row*self.csize[0]: (row+1)*self.csize[0]]
			print("".join(map(lambda x: '*' if x else ' ', row_data)) + "<", end="")
			val = 0
			for place in range(self.csize[0]):
				val += (2**place) * row_data[self.csize[0] - 1 - place]
			print(val)
			bytes.append(val)
		return bytes

	

def rle(data):
	out = []
	prev = -1
	i = 0
	while i < len(data):
		if data[i] != prev:
			prev = data[i]
			out.append(100+data[i])
		else:
			out[-1] += 100
		i += 1
	return out

def rle2(data):
	prev = -1
	out = []
	i = 0
	while i < len(data):
		if data[i] != prev:
			out.append(1)
			prev = data[i]
		else:
			out[-1] += 1
		i+=1
	return out

def words(data, bits):
	i=0
	wds = []
	w = 0
	while i < len(data):
		w += data[i]* (2** (i%bits));
		i += 1
		if i%bits == 0:
			wds.append(w)
			w = 0
	if i%bits:
		wds.append(w)
	tail = len(wds)-1
	while wds[tail] == 0:
		tail -= 1
	return wds[0:tail+1]

if __name__=="__main__":
	size = sys.argv[1][0:-4].split('_')
	font = FontFile(sys.argv[1], int(size[1]), int(size[2]))
	
	bits = font.get_bits()
	size = 32
	print("RLE2 %d  %f "%(len(bits), len(bits)/size))
	#~ print(rle2(bits))
	wds = words(bits, size)
	print(", 0x".join(map(lambda x: "%x"%x, wds)))
	print(len(wds))
	
	#~ tex = Image.new('1', (128,128) );
	
	#~ bytes = []
	#~ for c in range(32,127):#127-32):
		#~ cimg = font.get_ascii(c)
		#~ tex.paste(cimg, ((c%21)*font.csize[0], (c//21)*font.csize[1]))
	#~ print(bytes)
	#~ print("")
	#~ print(rle(bytes))
