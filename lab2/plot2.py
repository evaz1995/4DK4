import re
import matplotlib.pyplot as plt
import sys

#file_path = sys.argv[1]
file_path = "data_delay_vs_arrival_preemptive.txt"
mean_delay = []
arrival_rate = []
integer = re.compile(r"^[0-9]*")
floating = re.compile(r"\d*\.\d*")

f = open(file_path, "r")

while True:
	line = f.readline()
	if not line: break

	g = floating.findall(line)
	arrival_rate.append(float(g[0]))
	mean_delay.append(float(g[1]))
	

file_path = "voice_delay_vs_arrival_preemptive.txt"
mean_delay2 = []
arrival_rate2 = []

f = open(file_path, "r")
while True:
	line = f.readline()
	if not line: break
	g = floating.findall(line)
	arrival_rate2.append(float(g[0]))
	mean_delay2.append(float(g[1]))



plt.plot(arrival_rate, mean_delay,color = "blue", label = 'data delay')
plt.plot(arrival_rate2, mean_delay2,color = "orange", label = 'voice delay')
plt.legend()
plt.ylabel('mean delay(ms)')
plt.xlabel('arrival rate')
plt.title("arrival rate vs. mean delay\nsamples = 1e3 voice packets")
for x,y in zip(arrival_rate,mean_delay):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,10), # distance from text to points (x,y)
				 color = "blue",
                 ha='center') # horizontal alignment can be left, right or center
for x,y in zip(arrival_rate2,mean_delay2):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,-10), # distance from text to points (x,y)
				 color = "orange",
                 ha='center') # horizontal alignment can be left, right or center

plt.show()