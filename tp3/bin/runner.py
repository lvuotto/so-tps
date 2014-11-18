#!/usr/bin/python
from os import listdir
from os.path import join, isfile
from bson.code import Code
from pymongo.connection import MongoClient
import argparse

conn = MongoClient()
db = conn.reddit

EJS_DIRS = [ 'ej1' ]

def run(code, incoll = 'posts', outcoll = 'default'):
    print "Corriendo map_reduce sobre %s.%s" % ("reddit", incoll)
    for ej in  EJS_DIRS if code == 'all' else [ code ] :
        print "Algoritmos a ejecutar en recurso %s" % ej
        with open(join(ej, 'map.js')) as mapping_func_file:
            with open(join(ej, 'reduce.js')) as reduce_func_file:
                if isfile(join(ej,'finalize.js')):
                    finalize_func = Code(open(join(ej,'finalize.js')).read())
                else:
                    finalize_func = None
                collection = getattr(db,incoll)
                map_func = Code(mapping_func_file.read())
                reduce_func = Code(reduce_func_file.read())
                collection.map_reduce(map_func, reduce_func, outcoll, finalize = finalize_func)
        print "\tEl resultado de la operacion se encuentra almacenado en la colleccion %s." % ej
        print "\tEl contenido generado es %s " % " ".join([ str(x) for x in getattr(db, outcoll).find()])
       

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("code", default = 'all',
            help = "el directorio donde se encuentran los \
scripts de map y reduce, default corre todo los directorios ejx")
    parser.add_argument("--incoll",  default = 'posts',
            help = "la coleccion donde se ejecutara el mapReduce")
    parser.add_argument("--outcoll",  default = 'default',
            help = "la coleccion de salida del mapReduce")
    args = parser.parse_args()

    run(args.code, args.incoll, args.outcoll)

 



