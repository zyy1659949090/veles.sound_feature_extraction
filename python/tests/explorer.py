'''
Created on Mar 25, 2013

@author: Markovtsev Vadim <v.markovtsev@samsung.com>
'''


import logging
import unittest
from sound_feature_extraction.library import Library
from sound_feature_extraction.explorer import Explorer


class Test(unittest.TestCase):

    def setUp(self):
        logging.basicConfig(level=logging.DEBUG)

    def tearDown(self):
        pass

    def testExplorer(self):
        Library("/home/markhor/Development/SoundFeatureExtraction/build/src/"
                ".libs/libSoundFeatureExtraction.so")
        explorer = Explorer()
        for name in explorer.transforms:
            print(str(explorer.transforms[name]))
            print("")


if __name__ == "__main__":
    # import sys;sys.argv = ['', 'Test.testExplorer']
    unittest.main()
