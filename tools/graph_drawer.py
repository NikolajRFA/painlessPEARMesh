import json
import pandas as pd
import networkx as nx
import matplotlib.pyplot as plt


df = pd.read_excel('ESP32 MAC addresses.xlsx')
esp_map = dict(zip(df['NodeId'], df['ESP']))

def extract_unique_topologies(file_path):
    unique_topologies = []
    previous_topology = None

    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith("TOPOLOGY: "):
                topology_json = line.replace("TOPOLOGY: ", "").strip()
                topology_data = json.loads(topology_json)

                if topology_data != previous_topology:
                    unique_topologies.append(topology_data)
                    previous_topology = topology_data

    return unique_topologies



def build_undirected(node, G=None):
    if G is None:
        G = nx.Graph()
    nid = node['nodeId']
    G.add_node(nid, root=node.get('root', False))
    for child in node.get('subs', []):
        G.add_edge(nid, child['nodeId'])
        build_undirected(child, G)
    return G

for topology in extract_unique_topologies('data/20_2_1305_1.txt'):
    UG = build_undirected(topology)

    root = next(n for n,d in UG.nodes(data=True) if d['root'])


    directed = nx.DiGraph()
    for parent, child in nx.bfs_successors(UG, source=root):
        for c in child:
            directed.add_edge(c, parent)


    def hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5,
                      pos=None, parent=None):
        if pos is None:
            pos = {root:(xcenter, vert_loc)}
        else:
            pos[root] = (xcenter, vert_loc)
        nbrs = [n for n in G.neighbors(root) if n!=parent]
        if nbrs:
            dx = width/len(nbrs)
            nextx = xcenter - width/2 - dx/2
            for nbr in nbrs:
                nextx += dx
                pos = hierarchy_pos(G, nbr, width=dx, vert_gap=vert_gap,
                                    vert_loc=vert_loc-vert_gap,
                                    xcenter=nextx, pos=pos, parent=root)
        return pos

    pos = hierarchy_pos(UG, root)


    labels = {
        n: f"{esp_map.get(n,'?')}"
        for n in directed.nodes()
    }


    plt.figure(figsize=(8,6))
    nx.draw_networkx_nodes   (directed, pos, node_size=900)
    nx.draw_networkx_labels  (directed, pos, labels, font_size=10)
    nx.draw_networkx_edges   (
        directed, pos,
        arrows=True,
        arrowsize=20,
        arrowstyle='-|>'
    )
    plt.title("painless visualized")
    plt.axis('off')
    plt.tight_layout()
    plt.show()