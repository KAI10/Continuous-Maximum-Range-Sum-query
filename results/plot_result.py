# plot.py
 
# Created by: Ashik <ashik@KAI10>
# Created on: Wed, 04 Jan 2017

# Query Rectangle size: 0.01 x 0.01

import matplotlib.pyplot as plt

# without index
plt.plot([49, 3000, 5000], [0.296, 84, 375.434], 'b--')
plt.annotate('without Index', xy=(4400,300), xytext=(3000, 350), arrowprops=dict(facecolor='blue', shrink=0.05),)

# with index
plt.plot([49, 3000, 5000], [0.307, 41.6, 168.045], 'r--')
plt.annotate('with index', xy=(4750, 150), xytext=(3950, 200), arrowprops=dict(facecolor='red', shrink=0.05),)

plt.title('Experiment Results')
plt.axis([0, 5150, 0, 400])
plt.xlabel('NO. of objects')
plt.ylabel('execution time (seconds)')
plt.grid(True)

plt.show()

