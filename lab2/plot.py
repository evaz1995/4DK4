import re
import matplotlib.pyplot as plt
import sys

#file_path = sys.argv[1]
file_path = "delay_vs_arrival.txt"
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
	


plt.plot(arrival_rate, mean_delay)
plt.ylabel('mean delay(s)')
plt.xlabel('arrival rate')
plt.title("arrival rate vs. mean delay")
for x,y in zip(arrival_rate,mean_delay):
    label = "{:.2f}".format(y)
    plt.annotate(label, # this is the text
                 (x,y), # these are the coordinates to position the label
                 textcoords="offset points", # how to position the text
                 xytext=(0,5), # distance from text to points (x,y)
                 ha='center') # horizontal alignment can be left, right or center

plt.show()
