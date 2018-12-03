#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Xml文档生成类
"""
import xml.dom.minidom as dom
class XMLGenerator(object):
    """
    生成器
    """

    def __init__(self, xml_name):
        self.doc = dom.Document()
        self.xml_name = xml_name
        self.root_node = None

    def init_root(self, node_name):
        """
        初始根节点
        """
        self.root_node = self.create_node(node_name)
        self.doc.appendChild(self.root_node)
        return self.root_node

    def create_node(self, node_name, node_value=None):
        """
        生成一个节点并返回,有节点值就赋值
        """
        new_node = self.doc.createElement(node_name)
        if node_value is not None:
            self.set_node_value(new_node, node_value)
        return new_node

    def create_add_node(self, parent_node, node_name, node_value=None):
        """
        生成一个节点并返回,有节点值就赋值,并添加到父节点上
        """
        child = self.create_node(node_name, node_value)
        return self.add_node(child, parent_node)

    def add_node(self, node, prev_node=None):
        """
        添加一个节点到父节点上
        """
        if prev_node is not None:
            prev_node.appendChild(node)
        else:
            self.root_node.appendChild(node)
        return node

    def set_node_att(self, node, att_name, att_value):
        """
        设置节点的属性
        """
        node.setAttribute(att_name, att_value)

    def set_node_value(self, node, node_value):
        """
        设置节点的值
        """
        new_node = self.doc.createTextNode(node_value)
        node.appendChild(new_node)

    def gen_xml(self):
        """
        生成文件
        """
        f = open(self.xml_name, "w")
        f.write(self.doc.toprettyxml(indent="\t", newl="\n", encoding="utf-8"))
        f.close()