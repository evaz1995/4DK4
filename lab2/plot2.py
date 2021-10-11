import re
import matplotlib.pyplot as plt
import sys

#file_path = sys.argv[1]
file_path = "delay_vs_arrival_1s.txt"
mean_delay = []
arrival_rate = []
integer = re.compile(r"^[0-9]*")
floating = re.compile(r"\d*\.\d*")

f = open(file_path, "r")

while True:
	line = f.readline()
	if not line: break

	arrival_rate.append(float(integer.search(line).group(0)))
	mean_delay.append(float(floating.search(line).group(0)))
	

file_path = "delay_vs_arrival_2s.txt"
mean_delay2 = []
arrival_rate2 = []

f = open(file_path, "r")
while True:
	line = f.readline()
	if not line: break

	arrival_rate2.append(float(integer.search(line).group(0)))
	mean_delay2.append(float(floating.search(line).group(0)))



plt.plot(arrival_rate, mean_delay,color = "blue", label = '1 server')
plt.plot(arrival_rate2, mean_delay2,color = "orange", label = '2 server')
plt.legend()
plt.ylabel('mean delay(ms)')
plt.xlabel('arrival rate')
plt.title("arrival rate vs. mean delay\nsamples = 1e6")
for x,y in zip(arrival_rate,mean_delay):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,10), # distance from text to points (x,y)
				 color = "orange",
                 ha='center') # horizontal alignment can be left, right or center
for x,y in zip(arrival_rate2,mean_delay2):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,-10), # distance from text to points (x,y)
				 color = "b",
                 ha='center') # horizontal alignment can be left, right or center

plt.show()