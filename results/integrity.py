#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: March 26, 2017
# Comment: A script that checks the integrity of each result file

import os
import argparse
from utils import implementation


def build_expected_filelist(size, niters, directory):

    file_set = set()
    if implementation(directory) == 'AVX2' or 'AVX2 (ST)':
        for coref in range(12, 22):
            for ucoref in range(12, 29):
                for nthread in range(2, 25):
                    file_set.add((size+'_'+niters+'_c'+str(coref)+'_u'+str(ucoref)+'_t'+str(nthread)))
        return file_set

    else:
        for coref in range(12, 26):
            for ucoref in range(12, 31):
                for nthread in range(2, 25):
                    file_set.add((size+'_'+niters+'_c'+str(coref)+'_u'+str(ucoref)+'_t'+str(nthread)))
        return file_set


def main():

    parser = argparse.ArgumentParser(description='A tool that checks the integrity of each result file.')
    parser.add_argument('-s', '--size', help='Problem size.', required=True)
    parser.add_argument('-i', '--iterations', help='Number of iteration.', required=True)
    parser.add_argument('-d', '--directories', nargs='*', help='Result directories.', required=True)
    args = vars(parser.parse_args())

    for directory in args['directories']:
        fset = build_expected_filelist(args['size'], args['iterations'], directory)
        resfiles = os.listdir(directory)
        resset = set(resfiles)

        missing_files = set()
        empty_files = set()

        for file in fset:
            if not file in resset:
                missing_files.add(file)

            if file in resfiles:
                if os.stat(directory + '/' + file).st_size == 0:
                    empty_files.add(file)

        if len(missing_files) < 1:
            print 'No files were missing!'
        else:
            print 'The following files are missing: '
            for mfile in sorted(missing_files):
                print mfile
        if len(empty_files) > 0:
            print 'The following files are empty : ', empty_files
        else:
            print 'No files were empty!'


if __name__ == '__main__':
    main()
