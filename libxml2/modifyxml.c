// changXmlFile.cpp
 
#include <libxml/parser.h>
#include <iostream>
 
using namespace std;
 
int main(int argc, char* argv[])
{
    xmlDocPtr doc;   //定义解析文档指针
    xmlNodePtr curNode;  //定义结点指针(你需要它为了在各个结点间移动)
 
    char *szDocName;
    if (argc <= 1) 
    {
        printf("Usage: %s docname\n", argv[0]);
        return(0);
    }
    szDocName = argv[1];
 
    doc = xmlReadFile(szDocName,"GB2312",XML_PARSE_RECOVER);  //解析文件
    if (NULL == doc)
    {  
        fprintf(stderr,"Document not parsed successfully. \n");    
        return -1;
    }
 
    curNode = xmlDocGetRootElement(doc);
    if (NULL == curNode)
    {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }
 
    curNode = curNode->children;
    while (NULL != curNode)
    {
        //删除newNode1
        if (!xmlStrcmp(curNode->name, BAD_CAST "newNode1"))
        {
            xmlNodePtr tempNode;
            tempNode = curNode->next; //使用一个临时变量来存储断链节点的后续节点
            xmlUnlinkNode(curNode); //将当前节点从文档中断链（unlink），这样本文档就不会再包含这个子节点
            xmlFreeNode(curNode); //手动删除断链节点的内存
            curNode = tempNode;
            continue;
        }
 
        //修改node2的属性值
        if (!xmlStrcmp(curNode->name, BAD_CAST "node2"))
        {
            xmlSetProp(curNode,BAD_CAST "attribute", BAD_CAST "no");
        }
        //修改newNode2的内容
        if (!xmlStrcmp(curNode->name, BAD_CAST "newNode2"))
        {
            xmlNodeSetContent(curNode, BAD_CAST "content changed");
        }
        //增加一个属性
        if (!xmlStrcmp(curNode->name, BAD_CAST "newNode3"))
        {
            xmlNewProp(curNode, BAD_CAST "newAttr", BAD_CAST "YES");
        }
 
        //增加一个子节点
        if (!xmlStrcmp(curNode->name, BAD_CAST "son"))
        {
            xmlNewTextChild(curNode, NULL, BAD_CAST "newGrandSon", BAD_CAST "new content");
        }
 
        curNode = curNode->next;
    }
 
    //存储xml文档
    int nRel = xmlSaveFile("ChangedXml.xml",doc);
    if (nRel != -1)
    {
        cout<<"一个xml文档被创建,写入"<<nRel<<"个字节"<<endl;
    }
    
	xmlFreeDoc(doc);
    return 0;
 
}

