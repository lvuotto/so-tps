#!/usr/bin/env python 
from pymongo.connection import MongoClient
import csv
import json

""" Script para re-importar los posts a la base de datos """

reader = csv.DictReader(open('data/redit.csv'),
        fieldnames=('image_id','unixtime','rawtime','title','total_votes',
            'reddit_id','number_of_upvotes','subreddit','number_of_downvotes',
            'localtime','score','number_of_comments','username'))

conn = MongoClient() 
db = conn.reddit
print "Cleaning DB collections %s.%s" % ("reddit", "posts")
db.posts.remove()

for it in reader:
    try:
        it['total_votes'] = int(it['total_votes'] )
        it['number_of_upvotes'] = int(it['number_of_upvotes'] )
        it['number_of_downvotes'] = int(it['number_of_downvotes'] )
        it['score'] = int(it['score'] )
        it['number_of_comments'] = int(it['number_of_comments'] )
        db.posts.insert(it)
    except Exception as e:
        print e, "while inserting", it

print "Inserted %d records" % db.posts.count()
assert db.posts.count() == 269689, "Missing records on DB, inserted: %d" % db.posts.count()


