import os
import shutil
import fnmatch
import sys
import datetime

now = datetime.datetime.now()
src = '..'
dst = "../../Shahnarman_%s" % now.strftime("%Y-%m-%d")
ign = ('*.bak','.svn','*.ilk','*.pdb','*.lastbuildstate','*.manifest','*.rc','*.res','*.tlog','*.obj','*.idb','ipch','*.sdf','*.suo','*.user')

def my_ignore_patterns(*patterns):
	def _ignore_patterns(path, names):
		print 'Copying %s' % path
		ignored_names = []
		for pattern in patterns:
			ignored_names.extend(fnmatch.filter(names, pattern))
		return set(ignored_names)
	return _ignore_patterns

ign_call = my_ignore_patterns(*ign)

if os.path.exists(dst):
	print 'Destination path %s exists; delete it? (y/n)' % dst
	res = sys.stdin.readline()
	if res[0] == 'y':
		print 'Removing tree...'
		shutil.rmtree(dst)
		shutil.copytree(src, dst, False, ign_call)
else:
	shutil.copytree(src, dst, False, ign_call)
