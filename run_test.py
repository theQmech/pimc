import multiprocessing
from os import listdir
from os import system
from os.path import isfile, join
import tqdm

mypath = "../aigs/"
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]

def test_aigfile(fname):
	# print (fname)
	cmd = "build/pimc input.lre ../aigs/" + fname + " 1>results/" + fname + " 2>&1"
	system(cmd)

pool = multiprocessing.Pool(multiprocessing.cpu_count())
_ = list(tqdm.tqdm(pool.imap(test_aigfile, onlyfiles), total=300))
