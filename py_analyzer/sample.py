__author__ = 'liudy'

import networkx as nx
from operateDB import connectDB

def dataProcess(graph, collection):
    for e in graph.edges():
        newItem = {'source': e[0], 'target': e[1]}
        collection.insert(newItem)

if __name__ == "__main__":
    path = './gmldata/'
    fileName = 'power'
    graph = nx.read_gml(path + fileName + '.gml')
    collection = connectDB('egocf-dev',  fileName + '_edge')['col']
    dataProcess(graph, collection)