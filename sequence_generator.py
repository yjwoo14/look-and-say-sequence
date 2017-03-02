import sys
def ant(n):
	if n == 1:
		yield 1
		return;
	
	prev = 0
	count = 0
	for next in ant(n-1):
		if next == prev:
			count += 1
			continue
		if count > 0:
			yield count
			yield prev
		prev = next
		count = 1
	yield count
	yield prev

for x in ant(100):
	sys.stdout.write(str(x))
