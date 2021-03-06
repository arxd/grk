#!/usr/bin/python3
import sys
import struct
import json
import time
import os.path
import http.client
import urllib.parse
import math
from datetime import datetime
import traceback
import subprocess

class Kracken():
	def __init__(self):
		self.conn = http.client.HTTPSConnection('api.kraken.com')
		self.conn.connect()
		self.public = "https://api.kraken.com/0/public/"
		
	def close(self):
		self.conn.close()
	
	def call(self, end, **kwargs):
		data = urllib.parse.urlencode(kwargs)
		self.conn.request("POST", self.public+end, data)
		resp = self.conn.getresponse()
		if resp.status != 200:
			raise Exception("Server Error: %d"%resp.status)
		#print("Response Headers: "+repr(resp.getheaders()))
		jresp = json.loads(resp.read().decode("utf-8"))
		if jresp['error'] == ['EService:Unavailable']:
			raise ConnectionError()
		elif jresp['error']:
			raise Exception("Handling Error: %r\n"%jresp['error'])
		return jresp['result']

	def __enter__(self):
		return self

	def __exit__(self, type, value, tb):
		self.conn.close()


class Trades():
	TRADE = struct.Struct("ddd") # time, price, amount
	HEADER = struct.Struct("QQ") # N_trades, last
	
	def __init__(self, filename, pair = None, last=0, data=[]):
		self.last = last
		self.data = data
		self.pair = pair
		if not pair:
			self.pair = filename[0:filename.find('.')]
		self.filename = filename
		if filename:
			self.load(filename)

	def __len__(self):
		return len(self.data)
	
	def time(self, idx):
		return datetime.fromtimestamp(self.data[idx][0])
	
	def price(self, idx):
		return self.data[idx][1]
		
	def amount(self, idx):
		return self.data[idx][2]

	def write(self, filename=None):
		if not filename:
			filename = self.filename
		print ("Saving %d to '%s'"%(len(self), filename))
		with open(filename, 'wb') as f:
			hdr = (len(self.data), self.last)
			f.write(Trades.HEADER.pack(*hdr))
			for t in self.data:
				f.write(Trades.TRADE.pack(*t))
		print ("Wrote %d trades to %s @ %d"%(len(self), filename, self.last))
		

	def load(self, filename):
		if not os.path.exists(filename):
			print("File not found %s"%filename)
			
		elif filename.endswith('.bin'):
			with open(filename, 'rb') as f:
				hdr = Trades.HEADER.unpack(f.read(Trades.HEADER.size))
				self.last = hdr[1]
				print("Reading %d from binary"%hdr[0])
				for i in range(hdr[0]):
					self.data.append(Trades.TRADE.unpack(f.read(Trades.TRADE.size)))
					
		elif filename.endswith('.json'):
			with open(filename, 'r') as f:
				self.last = int(filename.split('_')[1][:-5])
				self.data = [(t[2], float(t[0]), float(t[1])*(-1.0 if t[3]=='s' else 1.0)) for t in json.load(f)]
			print("Read %d from json"%len(self))
			
		else:
			print("Unknown file %s"%filename)
			
		return self
		
	def merge(self, other):
		if other.last > self.last:
			self.last = other.last
		self.data += other.data
		self.data.sort(key=lambda x: x[0])
		# remove duplicates
		
		rem = 0
		dup = 0
		i = 0
		while i < len(self.data)-1:
			if self.data[i][0] == self.data[i+1][0]:
				if self.data[i][1] == self.data[i+1][1] and self.data[i][2] == self.data[i+1][2]:
					del self.data[i+1]
					rem += 1
					continue
				else:
					dup += 1
					self.data[i+1] = (self.data[i+1][0] + 0.0001, self.data[i+1][1], self.data[i+1][2])
			i += 1
		
		if dup:
			print("Shifted %d duplicates"%dup)
		if rem:
			print ("Removed %d duplicates"%rem)
	
	def try_update(self):
		prev_last = -1
		tprev = time.time()
		with Kracken() as k:
			while True:
				print("%d : %s"%(len(self), datetime.fromtimestamp(self.last*1.0e-9).strftime("%Y %b %d : %H")))
				if prev_last == self.last:
					raise StopIteration()
				prev_last = self.last
				resp = k.call('Trades', pair = self.pair, since=self.last)
				self.merge(Trades('', '', int(resp['last']), [(t[2], float(t[0]), float(t[1])*(-1.0 if t[3]=='s' else 1.0)) for t in resp[self.pair]] ))
				tafter = time.time()
				tsleep = 5.1-(tafter-tprev)
				if tsleep > 0.0:
					time.sleep(tsleep)
				tprev = tafter
	
	def update(self):
		try:
			self.try_update()
		except KeyboardInterrupt:
			print("Force Stop")
			self.write()
			return
		except StopIteration:
			print("Up to date")
			self.write()
			return
		except ConnectionError:
			print("Try again in 15 seconds...")
			self.write()
			time.sleep(15.0)
		except:
			print("Uh oh....")
			self.write()
			raise
		self.update()

if __name__ == '__main__':
	#t1 = Trades(sys.argv[1])
	
	def do_update(pair, filename, last=None):
		if last == None:
			proc = subprocess.Popen(['build/merge', 'last', filename], stdout=subprocess.PIPE)
			a,b = proc.communicate()
			last = int(a)

		print("UPDATE %s from %d"%(pair,last))
		
		t = Trades("", pair=pair, last=last, data=[])
		t.filename = '/tmp/ticker%s.bin'%pair
		t.update()
		print("DONE UPDATING")
		t.write()
		proc = subprocess.Popen(['build/merge', 'merge', filename, t.filename])
		proc.wait()
	
	
	if sys.argv[1] == 'update':
		pair = sys.argv[2][:sys.argv[2].find('.')]
		do_update(pair, sys.argv[2], None if len(sys.argv) == 3 else int(sys.argv[3]))
		
	elif sys.argv[1] == 'update_all':
		for f in os.listdir('.'):
			if not f.endswith('.bin'):
				continue
			do_update(f[:f.find('.')], f)
			time.sleep(5.0)

	elif sys.argv[1] == 'pairs':
		pairs = Kracken().call('AssetPairs')
		for p,v in pairs.items():
			
			print(p + " : %r"%v)
			print("---------")
			
	elif sys.argv[1] == 'show':
		pair = sys.argv[2]
		time = sys.argv[3]
		print("Get %s from %s"%(pair, time))
		data = Kracken().call('Trades', pair = pair, since=time)
		for t in data[pair]:
			print ("%f: %f  %f"%(t[2], float(t[0]), float(t[1])*(-1.0 if t[3]=='s' else 1.0)))