
'''
class ASTNode {
public:
  NodeType type;
  vector<ASTNode> children;
  nlohmann::json data;
  TokenMetadata metadata;
'''
import json
import sys
from typing import List, Dict, Any
from pprint import pprint

emit = print

class ASTNode:
    nodetype: str
    children: List[Any]
    data: Dict[Any, Any]

    def __init__(self, nodetype, children, data):
        self.nodetype = nodetype
        self.children = children
        self.data = data

def make_node_from_json(json_data) -> ASTNode:
    nodetype = json_data["type_string"]
    data = json_data["data"]
    children_raw = json_data["zchildren"]

    children = [
        make_node_from_json(child_raw) for child_raw in children_raw
    ]

    return ASTNode(nodetype, data, children)


def gen_node(node):
    match node.nodetype:
        case "TOP_LEVEL":
        case "FUNC_DECLARE":
        case "VAR_DECLARE":
        case "BLOCK":
        case "INT_LITERAL":
        case "STRING_LITERAL":
        case "FUNC_CALL":
        case "VAR_LOOKUP":
        case "EXPR_LIST":
        case _:
            raise ValueError("invalid node type")

if __name__ == "__main__":
    filename = sys.argv[1]
    data = None
    with open(filename) as json_data:
        data = json.loads(json_data.read())
        # json_data.close()
        # pprint(d)

    ast = make_node_from_json(data)
    gen_node(ast)
    

    