
#include <stdio.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#endif /*<FASTLOG_HAVE_LIBXML2>*/

int main()
{
    //定义文档和节点指针
    xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
    xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST"root");
    

    //设置根节点
    xmlDocSetRootElement(doc,root_node);
    
    //在根节点中直接创建节点
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode1", BAD_CAST "newNode1 content");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode2", BAD_CAST "newNode2 content");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode3", BAD_CAST "newNode3 content");
    
    //创建一个节点，设置其内容和属性，然后加入根结点
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"node2");
    xmlNodePtr content = xmlNewText(BAD_CAST"NODE CONTENT"); //注意不是xmlNewTextChild()
    xmlAddChild(root_node,node);
    xmlAddChild(node,content);
    xmlNewProp(node,BAD_CAST"attribute",BAD_CAST "yes");
    
    //创建一个儿子和孙子节点
    node = xmlNewNode(NULL, BAD_CAST "son");
    xmlAddChild(root_node,node);
    xmlNodePtr grandson = xmlNewNode(NULL, BAD_CAST "grandson");
    xmlAddChild(node,grandson);
    xmlAddChild(grandson, xmlNewText(BAD_CAST "This is a grandson node"));
    
    //存储xml文档
    //int nRel = xmlSaveFile("CreatedXml.xml",doc);
    int nRel = xmlSaveFormatFile("CreatedXml.xml",doc, 1);
    if (nRel != -1)
    {
       printf("write %d bytes.\n", nRel);
    }
    //释放文档内节点动态申请的内存
    xmlFreeDoc(doc);
    //<?xml version="1.0"?>
    //<root>
    //  <newNode1>newNode1 content</newNode1>
    //  <newNode2>newNode2 content</newNode2>
    //  <newNode3>newNode3 content</newNode3>
    //  <node2 attribute="yes">NODE CONTENT</node2>
    //  <son>
    //    <grandson>This is a grandson node</grandson>
    //  </son>
    //</root>
}

