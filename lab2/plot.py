import re
import matplotlib.pyplot as plt
import sys

#file_path = sys.argv[1]
file_path = "fix_delay_vs_arrival.txt"
mean_delay = []
prob = []
integer = re.compile(r"^[0-9]*")
floating = re.compile(r"\d*\.\d*")

f = open(file_path, "r")
while True:
	line = f.readline()
	if not line: break
	g = floating.findall(line)
	prob.append(float(g[0]))
	mean_delay.append(float(g[1]))
	


plt.plot(prob, mean_delay)
plt.ylabel('mean delay(s)')
plt.xlabel('mean arrival')
plt.title("mean arrival(data packet) vs. mean delay(voice packet)")
for x,y in zip(prob,mean_delay):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,5), # distance from text to points (x,y)
                 ha='center') # horizontal alignment can be left, right or center

plt.show()
