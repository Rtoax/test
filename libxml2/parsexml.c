// parserXmlFile.cpp
#include <libxml/parser.h>
#include <iostream>
 
using namespace std;
 
int main(int argc, char* argv[])
{
    xmlDocPtr doc;           //定义解析文档指针
    xmlNodePtr curNode;      //定义结点指针(你需要它为了在各个结点间移动)
    xmlChar *szKey;          //临时字符串变量
    char *szDocName;
    if (argc <= 1) 
    {
       printf("Usage: %s docname\n", argv[0]);
       return(0);
    }
 
    szDocName = argv[1];
    doc = xmlReadFile(szDocName,"GB2312",XML_PARSE_RECOVER); //解析文件
    //检查解析文档是否成功，如果不成功，libxml将指一个注册的错误并停止。
    //一个常见错误是不适当的编码。XML标准文档除了用UTF-8或UTF-16外还可用其它编码保存。
    //如果文档是这样，libxml将自动地为你转换到UTF-8。更多关于XML编码信息包含在XML标准中.
    if (NULL == doc)
    {  
       fprintf(stderr,"Document not parsed successfully. \n");    
       return -1;
    }
 
    curNode = xmlDocGetRootElement(doc); //确定文档根元素
 
    if (NULL == curNode)
    {
       fprintf(stderr,"empty document\n");
       xmlFreeDoc(doc);
       return -1;
    }
 
    if (xmlStrcmp(curNode->name, BAD_CAST "root"))
    {
       fprintf(stderr,"document of the wrong type, root node != root");
       xmlFreeDoc(doc);
       return -1;
    }
    curNode = curNode->xmlChildrenNode; //子节点集是链表
    xmlNodePtr propNodePtr = curNode;
    while(curNode != NULL)
    {
       //取出节点中的内容
       if (!xmlStrcmp(curNode->name, (const xmlChar *)"newNode"))
       {
           szKey = xmlNodeGetContent(curNode);
           printf("newNode1: %s\n", szKey);
           xmlFree(szKey);
       }
 
       //查找带有属性attribute的节点
       if (xmlHasProp(curNode,BAD_CAST "attribute"))
       {
           propNodePtr = curNode;
       }
       curNode = curNode->next;
    }
 
    //查找属性
    xmlAttrPtr attrPtr = propNodePtr->properties; //属性集是链表
    while (attrPtr != NULL)
    {
       if (!xmlStrcmp(attrPtr->name, BAD_CAST "attribute"))
       {
           xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST "attribute");
   //szAttr要调用函数xmlFree(szAttr)手动删除否则会发生内存泄露。
           cout<<"get attribute = "<<szAttr<<endl;
           xmlFree(szAttr);
       }
       attrPtr = attrPtr->next;
    }
 
    xmlFreeDoc(doc);
    return 0;
 
}

