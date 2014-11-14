#!/usr/bin/env python
import os
""" Script para reinicializar la base de mongo, en caso de que se corrompiera"""
os.remove('/var/lib/mongodb/mongod.lock')
os.system('service mongodb restart')

