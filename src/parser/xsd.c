/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
 copyright   : (C) 2003 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG */

#include "xsd_p.h"
#include "xml_l.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"
#include "gwenhywfar/text.h"
#include "gwenhywfar/bio_buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <errno.h>
#include <unistd.h>


#include "xsdtypes.inc"


GWEN_LIST_FUNCTIONS(GWEN_XSD_NAMESPACE, GWEN_XSD_NameSpace)
GWEN_LIST_FUNCTIONS(GWEN_XSD_FACETS, GWEN_XSD_Facets)




GWEN_XSD_NAMESPACE *GWEN_XSD_NameSpace_new(const char *id,
					   const char *name,
					   const char *url,
					   const char *localUrl) {
  GWEN_XSD_NAMESPACE *ns;

  assert(id);
  assert(name);
  GWEN_NEW_OBJECT(GWEN_XSD_NAMESPACE, ns);
  GWEN_LIST_INIT(GWEN_XSD_NAMESPACE, ns);
  ns->id=strdup(id);
  ns->name=strdup(name);
  if (url)
    ns->url=strdup(url);
  if (localUrl)
    ns->localUrl=strdup(localUrl);
  ns->xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "namespace");
  return ns;
}



void GWEN_XSD_NameSpace_free(GWEN_XSD_NAMESPACE *ns){
  if (ns) {
    free(ns->localUrl);
    free(ns->url);
    free(ns->id);
    GWEN_XMLNode_free(ns->xmlNode);
    GWEN_LIST_FINI(GWEN_XSD_NAMESPACE, ns);
    GWEN_FREE_OBJECT(ns);
  }
}



GWEN_XSD_FACETS *GWEN_XSD_Facets_new(){
  GWEN_XSD_FACETS *xf;

  GWEN_NEW_OBJECT(GWEN_XSD_FACETS, xf);
  GWEN_LIST_INIT(GWEN_XSD_FACETS, xf);
  xf->enumeration=GWEN_StringList_new();
  xf->pattern=GWEN_StringList_new();

  return xf;
}



void GWEN_XSD_Facets_free(GWEN_XSD_FACETS *xf){
  if (xf) {
    free(xf->baseType);
    GWEN_StringList_free(xf->enumeration);
    GWEN_StringList_free(xf->pattern);
    GWEN_LIST_FINI(GWEN_XSD_FACETS, xf);
    GWEN_FREE_OBJECT(xf);
  }
}








GWEN_XSD_ENGINE *GWEN_XSD_new() {
  GWEN_XSD_ENGINE *e;

  GWEN_NEW_OBJECT(GWEN_XSD_ENGINE, e);
  e->rootNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  e->nameSpaces=GWEN_XSD_NameSpace_List_new();
  e->userNameSpaces=GWEN_XSD_NameSpace_List_new();

  return e;
}



void GWEN_XSD_free(GWEN_XSD_ENGINE *e){
  if (e) {
    free(e->currentTargetNameSpace);
    GWEN_XMLNode_free(e->rootNode);
    GWEN_XSD_NameSpace_List_free(e->userNameSpaces);
    GWEN_XSD_NameSpace_List_free(e->nameSpaces);
    GWEN_FREE_OBJECT(e);
  }
}




GWEN_XSD_NAMESPACE *GWEN_XSD__FindNameSpaceById(GWEN_XSD_ENGINE *e,
						const char *name) {
  GWEN_XSD_NAMESPACE *ns;

  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  while(ns) {
    if (strcasecmp(name, ns->id)==0)
      break;
    ns=GWEN_XSD_NameSpace_List_Next(ns);
  }

  return ns;
}



GWEN_XSD_NAMESPACE *GWEN_XSD__FindNameSpaceByName(GWEN_XSD_ENGINE *e,
						 const char *name) {
  GWEN_XSD_NAMESPACE *ns;

  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  while(ns) {
    if (strcasecmp(name, ns->name)==0)
      break;
    ns=GWEN_XSD_NameSpace_List_Next(ns);
  }

  return ns;
}



int GWEN_XSD__RemoveNamespace(GWEN_XSD_ENGINE *e,
			      const char *xsdPrefix,
			      GWEN_XMLNODE *xmlNode) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while(n) {
    const char *s;
    const char *p;

    s=GWEN_XMLNode_GetData(n);
    assert(s);
    p=strchr(s, ':');
    if (p) {
      if (xsdPrefix) {
	if (strncasecmp(s, xsdPrefix, p-s)==0) {
	  char *save;

	  save=strdup(p+1);
	  /* remove name space name */
	  DBG_INFO(GWEN_LOGDOMAIN, "Removing namespace from tag \"%s\"", s);
	  GWEN_XMLNode_SetData(n, save);
	  free(save);
	}
	else {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Bad namespace in schema file (%s)", s);
	  return -1;
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad namespace in schema file (%s)", s);
	return -1;
      }
    }

    GWEN_XSD__RemoveNamespace(e, xsdPrefix, n);

    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}




int GWEN_XSD__ExchangeNamespaceOnProperty(GWEN_XSD_ENGINE *e,
					  const char *name,
					  const char *newPrefix,
					  const char *oldPrefix,
					  GWEN_XMLNODE *n) {
  const char *pname;
  
  pname=GWEN_XMLNode_GetProperty(n, name, 0);
  if (pname) {
    const char *p;
  
    p=strchr(pname, ':');
    if (p) {
      if (newPrefix) {
	if (strncasecmp(pname, newPrefix, p-pname)==0) {
	  GWEN_BUFFER *tbuf;
  
	  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	  GWEN_Buffer_AppendString(tbuf, oldPrefix);
	  GWEN_Buffer_AppendByte(tbuf, ':');
	  GWEN_Buffer_AppendString(tbuf, p+1);
  
	  /* convert name space name */
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Changing namespace for tag \"%s\"",
		   GWEN_XMLNode_GetData(n));
	  GWEN_XMLNode_SetProperty(n, name, GWEN_Buffer_GetStart(tbuf));
	  GWEN_Buffer_free(tbuf);
	}
      } /* if newPrefix */
    } /* if property contains a colon */
    else {
      if (newPrefix==0) {
	GWEN_BUFFER *tbuf;
  
	tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	GWEN_Buffer_AppendString(tbuf, oldPrefix);
	GWEN_Buffer_AppendByte(tbuf, ':');
	GWEN_Buffer_AppendString(tbuf, pname);
  
	/* convert name space name */
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Changing namespace for tag \"%s\"",
		 GWEN_XMLNode_GetData(n));
	GWEN_XMLNode_SetProperty(n, name, GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_free(tbuf);
      }
    }
  } /* if property exists */
  return 0;
}



int GWEN_XSD__ExchangeNamespace(GWEN_XSD_ENGINE *e,
				const char *newPrefix,
				const char *oldPrefix,
				GWEN_XMLNODE *xmlNode,
				int refOnly) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while(n) {
    const char *s;

    s=GWEN_XMLNode_GetData(n);
    assert(s);

    if (strcasecmp(s, "complexType")==0 ||
	strcasecmp(s, "simpleType")==0) {
      if (!refOnly) {
	if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "name",
						  newPrefix,
						  oldPrefix,
						  n)) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return -1;
	}
      }
    }
    else if (strcasecmp(s, "element")==0) {
      if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "type",
						newPrefix,
						oldPrefix,
						n)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
      if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "ref",
						newPrefix,
						oldPrefix,
						n)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
      if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "substitutionGroup",
						newPrefix,
						oldPrefix,
						n)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
      if (!refOnly) {
	if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "name",
						  newPrefix,
						  oldPrefix,
						  n)) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return -1;
	}
      }
    }
    else if (strcasecmp(s, "restriction")==0 ||
	     strcasecmp(s, "extension")==0) {
      if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "base",
						newPrefix,
						oldPrefix,
						n)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
    }
    else if (strcasecmp(s, "group")==0) {
      if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "ref",
						newPrefix,
						oldPrefix,
						n)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
      if (!refOnly) {
	if (GWEN_XSD__ExchangeNamespaceOnProperty(e, "name",
						  newPrefix,
						  oldPrefix,
						  n)) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return -1;
	}
      }
    }

    GWEN_XSD__ExchangeNamespace(e, newPrefix, oldPrefix, n, refOnly);

    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int GWEN_XSD__ImportSchema(GWEN_XSD_ENGINE *e,
			   GWEN_XMLNODE *xmlNode,
			   const char *fname) {
  GWEN_XSD_NAMESPACE *docNs;
  GWEN_XMLPROPERTY *pr;
  const char *nameSpace;
  const char *xsNs;
  const char *p;
  GWEN_XMLNODE *fn;

  /* get the schema namespace and remove it from all tags to make the
   * document easier to handle
   */
  xsNs=GWEN_XMLNode_GetData(xmlNode);
  assert(xsNs);
  p=strchr(xsNs, ':');
  if (p) {
    char *newNs;
    int rv;

    if (strcasecmp(p+1, "schema")!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Document is not a XML-schema.");
      return -1;
    }
    newNs=(char*)malloc(p-xsNs+1);
    assert(newNs);
    memmove(newNs, xsNs, p-xsNs);
    newNs[xsNs-p]=0;
    rv=GWEN_XSD__RemoveNamespace(e, newNs, xmlNode);
    free(newNs);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad document");
      return -1;
    }
  }

  /* get global namespace of the document */
  nameSpace=GWEN_XMLNode_GetProperty(xmlNode, "targetNamespace", 0);
  if (!nameSpace)
    nameSpace=GWEN_XMLNode_GetProperty(xmlNode, "xmlns", 0);
  if (!nameSpace) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No namespace given in document");
    return -1;
  }

  /* find or create namespace object for this document */
  docNs=GWEN_XSD__FindNameSpaceByName(e, nameSpace);
  if (!docNs) {
    char idbuf[32];

    snprintf(idbuf, sizeof(idbuf), "_ns%d", ++(e->nextNameSpaceId));
    docNs=GWEN_XSD_NameSpace_new(idbuf, nameSpace, 0, 0);
    GWEN_XSD_NameSpace_List_Add(docNs, e->nameSpaces);
  }

  p=GWEN_XMLNode_GetProperty(xmlNode, "xmlns", 0);
  if (p) {
    GWEN_XSD_NAMESPACE *ns;

    ns=GWEN_XSD__FindNameSpaceByName(e, p);
    assert(ns);
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Converting namespace for global references to \"%s\"",
	      ns->id);
    if (GWEN_XSD__ExchangeNamespace(e, 0, ns->id, xmlNode, 1)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not convert global namespace to \"%s\"",
		ns->id);
      return -1;
    }
  }

  DBG_ERROR(GWEN_LOGDOMAIN,
	    "Converting global namespace to \"%s\"",
	    docNs->id);

  /* make the document uses the new prefix */
  if (GWEN_XSD__ExchangeNamespace(e, 0, docNs->id, xmlNode, 0)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not convert global namespace to \"%s\"",
	      docNs->id);
    return -1;
  }

  /* extract additional namespaces */
  pr=GWEN_XMLNode_GetFirstProperty(xmlNode);
  while(pr) {
    const char *name;

    name=GWEN_XMLProperty_GetName(pr);
    assert(name);
    if (strncasecmp(name, "xmlns:", 6)==0) {
      GWEN_XSD_NAMESPACE *ns;
      const char *pname;
      const char *newPrefix;
      const char *oldPrefix;

      newPrefix=name+6;
      oldPrefix=0;
      pname=GWEN_XMLProperty_GetValue(pr);
      ns=GWEN_XSD__FindNameSpaceByName(e, pname);
      if (ns)
	oldPrefix=ns->id;
      else {
	char idbuf[32];

	snprintf(idbuf, sizeof(idbuf), "_ns%d", ++(e->nextNameSpaceId));
	ns=GWEN_XSD_NameSpace_new(idbuf, pname, 0, 0);
	GWEN_XSD_NameSpace_List_Add(ns, e->nameSpaces);
	oldPrefix=ns->id;
      }
      assert(oldPrefix);

      DBG_ERROR(GWEN_LOGDOMAIN,
		"Converting namespace \"%s\" to \"%s\"",
		newPrefix, oldPrefix);

      /* make the document use the new prefix */
      if (GWEN_XSD__ExchangeNamespace(e, newPrefix, oldPrefix, xmlNode, 0)) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Could not convert namespace \"%s\" to \"%s\"",
		  newPrefix, oldPrefix);
	return -1;
      }

    } /* if namespace */

    pr=GWEN_XMLNode_GetNextProperty(xmlNode, pr);
  } /* while property */

  /* add schema to root */
  fn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "file");
  GWEN_XMLNode_SetProperty(fn, "filename", fname);
  GWEN_XMLNode_SetProperty(fn, "namespaceprefix", docNs->id);
  GWEN_XMLNode_SetProperty(fn, "filenamespace", docNs->name);
  /* copy some important properties */
  p=GWEN_XMLNode_GetProperty(xmlNode, "elementFormDefault", 0);
  if (p)
    GWEN_XMLNode_SetProperty(fn, "elementFormDefault", p);
  p=GWEN_XMLNode_GetProperty(xmlNode, "attributeFormDefault", 0);
  if (p)
    GWEN_XMLNode_SetProperty(fn, "attributeFormDefault", p);
  GWEN_XMLNode_AddChildrenOnly(fn,
			       xmlNode,
			       1 /* copyThem */);
  GWEN_XMLNode_AddChild(e->rootNode, fn);
  if (GWEN_XMLNode_GetParent(xmlNode))
    GWEN_XMLNode_UnlinkChild(GWEN_XMLNode_GetParent(xmlNode), xmlNode);
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



int GWEN_XSD_ImportSchema(GWEN_XSD_ENGINE *e,
			  GWEN_XMLNODE *xmlNode,
			  const char *fname) {
  GWEN_XMLNODE *n;
  int rv;

  assert(e);
  assert(xmlNode);

  if (e->derivedTypesImported==0) {
    e->derivedTypesImported=1;
    if (GWEN_XSD__ImportDerivedTypes(e)) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return -1;
    }
  }

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while(n) {
    const char *s;
    const char *p;

    s=GWEN_XMLNode_GetData(n);
    assert(s);
    p=strchr(s, ':');
    if (p) {
      if (strcasecmp(p+1, "schema")==0)
        break;
    }
    else {
      if (strcasecmp(s, "schema")==0)
	break;
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }

  if (!n) {
    DBG_ERROR(GWEN_LOGDOMAIN, "XML node does not contain a XML schema");
    return -1;
  }

  rv=GWEN_XSD__ImportSchema(e, n, fname);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



int GWEN_XSD__FinishNode(GWEN_XSD_ENGINE *e, GWEN_XMLNODE *n) {
  GWEN_XMLNODE *nn;
  const char *tag;
  const char *tName;
  const char *tType;

  tName=GWEN_XMLNode_GetProperty(n, "name", 0);
  tag=GWEN_XMLNode_GetData(n);
  assert(tag);
  tName=GWEN_XMLNode_GetProperty(n, "name", 0);
  tType=GWEN_XMLNode_GetProperty(n, "type", 0);

  if (strcasecmp(tag, "element")==0) {
    if (!GWEN_XMLNode_GetProperty(n, "ref", 0)) {
      if (tType) {
        GWEN_XMLNODE *nType;
        const char *x;

        nType=GWEN_XSD_GetTypeNode(e, tType);
        if (nType) {
          x=GWEN_XMLNode_GetData(nType);
          assert(x);
          if (strcasecmp(x, "complexType")==0)
            GWEN_XMLNode_SetProperty(n, "_isComplex", "1");
        }
      }
      else {
        GWEN_XMLNODE *n1;

        /* type is declared inside element, check for type */
        n1=GWEN_XMLNode_FindFirstTag(n, "complexType", 0, 0);
        if (n1) {
          if (!GWEN_XMLNode_FindFirstTag(n1, "simpleContent", 0, 0))
            GWEN_XMLNode_SetProperty(n, "_isComplex", "1");
        }
      }
    } /* if !ref */
  }

  /* handle all children */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    int rv;
    GWEN_XMLNODE *nnNext;
    const char *s;

    nnNext=GWEN_XMLNode_GetNextTag(nn);
    s=GWEN_XMLNode_GetData(nn);
    assert(s);
    if (strcasecmp(s, "annotation")==0) {
      GWEN_XMLNode_UnlinkChild(n, nn);
      GWEN_XMLNode_free(nn);
    }
    else if (strcasecmp(s, "documentation")==0) {
      GWEN_XMLNode_UnlinkChild(n, nn);
      GWEN_XMLNode_free(nn);
    }
    else {
      rv=GWEN_XSD__FinishNode(e, nn);
      if (rv)
        return rv;
    }
    nn=nnNext;
  }

  return 0;
}



int GWEN_XSD__FinishXsdDoc(GWEN_XSD_ENGINE *e) {
  return GWEN_XSD__FinishNode(e, e->rootNode);
}



GWEN_XMLNODE *GWEN_XSD_GetTypeNode(GWEN_XSD_ENGINE *e,
                                   const char *name) {
  GWEN_XMLNODE *nFile;

  nFile=GWEN_XMLNode_FindFirstTag(e->rootNode, "file", 0, 0);
  while(nFile) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(nFile, "complexType", "name", name);
    if (!n)
      n=GWEN_XMLNode_FindFirstTag(nFile, "simpleType", "name", name);
    if (n)
      return n;

    nFile=GWEN_XMLNode_FindNextTag(nFile, "file", 0, 0);
  }

  return 0;
}



GWEN_XMLNODE *GWEN_XSD_GetElementNode(GWEN_XSD_ENGINE *e,
				      const char *name) {
  GWEN_XMLNODE *nFile;

  nFile=GWEN_XMLNode_FindFirstTag(e->rootNode, "file", 0, 0);
  while(nFile) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(nFile, "element", "name", name);
    if (n)
      return n;

    nFile=GWEN_XMLNode_FindNextTag(nFile, "file", 0, 0);
  }

  return 0;
}



GWEN_XMLNODE *GWEN_XSD_GetGroupNode(GWEN_XSD_ENGINE *e,
				    const char *name) {
  GWEN_XMLNODE *nFile;

  nFile=GWEN_XMLNode_FindFirstTag(e->rootNode, "file", 0, 0);
  while(nFile) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(nFile, "group", "name", name);
    if (n)
      return n;

    nFile=GWEN_XMLNode_FindNextTag(nFile, "file", 0, 0);
  }

  return 0;
}



int GWEN_XSD__GetTypeFacets(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *nType,
			    GWEN_XSD_FACETS *xf){
  GWEN_XMLNODE_LIST2 *nl;
  GWEN_XMLNODE_LIST2_ITERATOR *nit;
  const char *name;
  GWEN_XMLNODE *n0;

  nl=GWEN_XMLNode_List2_new();
  name=GWEN_XMLNode_GetProperty(nType, "name", 0);

  /* sample all base types */
  n0=nType;
  while(n0) {
    GWEN_XMLNode_List2_PushFront(nl, n0);
    if (n0) {
      GWEN_XMLNODE *n2;

      n2=GWEN_XMLNode_FindFirstTag(n0, "restriction", 0, 0);
      if (!n2)
        n2=GWEN_XMLNode_FindFirstTag(n0, "extension", 0, 0);
      if (n2) {
        const char *s;

        s=GWEN_XMLNode_GetProperty(n2, "base", 0);
        if (!s)
          s=GWEN_XMLNode_GetProperty(n2, "substituteGroup", 0);
        if (!s)
          break;
        n0=GWEN_XSD_GetTypeNode(e, s);
        if (!n0) {
          DBG_INFO(GWEN_LOGDOMAIN, "Node for type \"%s\" not found", s);
        }
      }
    }
    else
      break;
  } /* while */

  /* now we have all types together */
  nit=GWEN_XMLNode_List2_First(nl);
  if (nit) {
    GWEN_XMLNODE *lxmlNode;

    lxmlNode=GWEN_XMLNode_List2Iterator_Data(nit);
    assert(lxmlNode);
    while(lxmlNode) {
      GWEN_XMLNODE *rnode;

      rnode=GWEN_XMLNode_FindFirstTag(lxmlNode, "restriction", 0, 0);
      if (rnode) {
	if (xf->baseType==0) {
          const char *baseName;

          baseName=GWEN_XMLNode_GetProperty(rnode, "base", 0);
          assert(baseName);
          xf->baseType=strdup(baseName);
        }

	rnode=GWEN_XMLNode_GetFirstTag(rnode);
	while(rnode) {
	  if (GWEN_XMLNode_GetType(rnode)==GWEN_XMLNodeTypeTag) {
	    const char *s;
	    const char *d;
	    double ivalue;
            int ivalueValid;
            int error;
            int isFixed;

	    error=0;
            ivalueValid=0;
            ivalue=0.0;
            s=GWEN_XMLNode_GetData(rnode);
	    assert(s);
            isFixed=(strcasecmp(GWEN_XMLNode_GetProperty(rnode,
                                                         "fixed",
                                                         "false"),
                                "true")==0);
            d=GWEN_XMLNode_GetProperty(rnode, "value", 0);
	    if (d)
	      ivalueValid=(1==sscanf(d, "%lf", &ivalue));

	    if (strcasecmp(s, "length")==0) {
              assert(ivalue);
              if (xf->fixedMask & GWEN_XSD_FACET_LENGTH) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_LENGTH) &&
                  xf->length!=ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
	      }
              else {
                xf->length=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_LENGTH;
                xf->setMask|=GWEN_XSD_FACET_LENGTH;
              }
	    }
	    else if (strcasecmp(s, "minlength")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MINLENGTH) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MINLENGTH) &&
                  xf->minLength>ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
              }
              else {
                xf->minLength=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MINLENGTH;
                xf->setMask|=GWEN_XSD_FACET_MINLENGTH;
              }
	    }
	    else if (strcasecmp(s, "maxlength")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MAXLENGTH) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MAXLENGTH) &&
                  xf->maxLength<ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
              }
              else {
                xf->maxLength=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MAXLENGTH;
                xf->setMask|=GWEN_XSD_FACET_MAXLENGTH;
              }
	    }
            else if (strcasecmp(s, "whiteSpace")==0) {
              assert(d);
              if (xf->fixedMask & GWEN_XSD_FACET_WHITESPACE) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if (strcasecmp(d, "preserve")==0)
                xf->whiteSpaceMode=GWEN_XSD_FacetWm_Preserve;
              else if (strcasecmp(d, "replace")==0)
                xf->whiteSpaceMode=GWEN_XSD_FacetWm_Replace;
              else if (strcasecmp(d, "collapse")==0)
                xf->whiteSpaceMode=GWEN_XSD_FacetWm_Collapse;
              else {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Unknown whitespace mode \"%s\"", d);
                error=1;
              }
              if (isFixed)
                xf->fixedMask|=GWEN_XSD_FACET_WHITESPACE;
              xf->setMask|=GWEN_XSD_FACET_WHITESPACE;
            }
	    else if (strcasecmp(s, "maxInclusive")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MAXINCLUSIVE) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MAXINCLUSIVE) &&
                  xf->maxInclusive>ivalue) {
		DBG_ERROR(GWEN_LOGDOMAIN,
			  "Bad value for restriction (set in lower level)");
		error=1;
	      }
              else {
		xf->maxInclusive=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MAXINCLUSIVE;
                xf->setMask|=GWEN_XSD_FACET_MAXINCLUSIVE;
              }
	    }
	    else if (strcasecmp(s, "maxExclusive")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MAXEXCLUSIVE) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MAXEXCLUSIVE) &&
                  xf->maxExclusive>ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
			  "Bad value for restriction (set in lower level)");
		error=1;
	      }
              else {
		xf->maxExclusive=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MAXEXCLUSIVE;
                xf->setMask|=GWEN_XSD_FACET_MAXEXCLUSIVE;
              }
	    }
	    else if (strcasecmp(s, "minInclusive")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MININCLUSIVE) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MININCLUSIVE) &&
                  xf->minInclusive>ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
              }
              else {
		xf->minInclusive=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MININCLUSIVE;
                xf->setMask|=GWEN_XSD_FACET_MININCLUSIVE;
              }
	    }
	    else if (strcasecmp(s, "minExclusive")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_MINEXCLUSIVE) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_MINEXCLUSIVE) &&
                  xf->minExclusive>ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
			  "Bad value for restriction (set in lower level)");
		error=1;
	      }
              else {
                xf->minExclusive=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_MINEXCLUSIVE;
                xf->setMask|=GWEN_XSD_FACET_MINEXCLUSIVE;
              }
	    }
	    else if (strcasecmp(s, "totalDigits")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_TOTALDIGITS) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_TOTALDIGITS) &&
                  xf->totalDigits<ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
              }
              else {
                xf->totalDigits=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_TOTALDIGITS;
                xf->setMask|=GWEN_XSD_FACET_TOTALDIGITS;
              }
	    }
	    else if (strcasecmp(s, "fractionDigits")==0) {
	      assert(ivalueValid);
              if (xf->fixedMask & GWEN_XSD_FACET_FRACTIONDIGITS) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Facet has already been fixed in lower level");
                error=1;
              }
              if ((xf->setMask & GWEN_XSD_FACET_FRACTIONDIGITS) &&
                  xf->fractionDigits<ivalue) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Bad value for restriction (set in lower level)");
                error=1;
              }
              else {
		xf->fractionDigits=ivalue;
                if (isFixed)
                  xf->fixedMask|=GWEN_XSD_FACET_FRACTIONDIGITS;
                xf->setMask|=GWEN_XSD_FACET_FRACTIONDIGITS;
              }
	    }
	    else if (strcasecmp(s, "enumeration")==0) {
	      assert(d);
	      GWEN_StringList_AppendString(xf->enumeration, d, 0, 1);
              if (isFixed)
                xf->fixedMask|=GWEN_XSD_FACET_ENUMERATION;
              xf->setMask|=GWEN_XSD_FACET_ENUMERATION;
	    }
	    else if (strcasecmp(s, "pattern")==0) {
	      assert(d);
	      GWEN_StringList_AppendString(xf->pattern, d, 0, 1);
              if (isFixed)
                xf->fixedMask|=GWEN_XSD_FACET_PATTERN;
              xf->setMask|=GWEN_XSD_FACET_PATTERN;
            }

	    if (error) {
	      GWEN_BUFFER *xpath;

	      xpath=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_XMLNode_GetXPath(e->rootNode, rnode, xpath);
	      DBG_ERROR(GWEN_LOGDOMAIN, "Error in \"%s\"",
			GWEN_Buffer_GetStart(xpath));
	      GWEN_Buffer_free(xpath);
	      GWEN_XMLNode_List2Iterator_free(nit);
	      GWEN_XMLNode_List2_free(nl);
	      return -1;
	    }
	  }
	  rnode=GWEN_XMLNode_GetNextTag(rnode);
	}
      }
      lxmlNode=GWEN_XMLNode_List2Iterator_Next(nit);
    }
    GWEN_XMLNode_List2Iterator_free(nit);
  } /* if nit */
  GWEN_XMLNode_List2_free(nl);

  return 0;
}



const char *GWEN_XSD_GetCurrentTargetNameSpace(const GWEN_XSD_ENGINE *e){
  assert(e);
  return e->currentTargetNameSpace;
}



int GWEN_XSD_SetCurrentTargetNameSpace(GWEN_XSD_ENGINE *e, const char *s){
  GWEN_XSD_NAMESPACE *ns;

  assert(e);
  ns=GWEN_XSD__FindNameSpaceByName(e, s);
  if (!ns) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown namespace \"%s\"", s);
    return -1;
  }
  free(e->currentTargetNameSpace);
  if (ns->id)
    e->currentTargetNameSpace=strdup(ns->id);
  else
    e->currentTargetNameSpace=0;
  return 0;
}



int GWEN_XSD__FinalizeNameSpaces(GWEN_XSD_ENGINE *e) {
  GWEN_XSD_NAMESPACE *ns1;

  assert(e);
  ns1=GWEN_XSD_NameSpace_List_First(e->userNameSpaces);
  while(ns1) {
    GWEN_XSD_NAMESPACE *ns2;
    GWEN_XSD_NAMESPACE *ns1n;

    ns1n=GWEN_XSD_NameSpace_List_Next(ns1);
    GWEN_XSD_NameSpace_List_Del(ns1);
    if (GWEN_XSD__FindNameSpaceByName(e, ns1->id)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "A namespace with prefix \"%s\" already exists",
                ns1->id);
      GWEN_XSD_NameSpace_free(ns1);
    }
    else {
      ns2=GWEN_XSD__FindNameSpaceByName(e, ns1->name);
      if (ns2) {
        if (ns1->id && ns2->id) {
          GWEN_XSD__ExchangeNamespace(e, ns2->id, ns1->id, e->rootNode, 0);
          free(ns2->id);
          ns2->id=strdup(ns1->id);
        }
        GWEN_XSD_NameSpace_free(ns1);
      }
      else {
        GWEN_XSD_NameSpace_List_Add(ns1, e->nameSpaces);
      }
    }
    ns1=ns1n;
  }
  return 0;
}



int GWEN_XSD__ImportDerivedTypes(GWEN_XSD_ENGINE *e) {
  GWEN_XMLNODE *tNode;

  assert(e);
  tNode=GWEN_XML_fromString(gwen_xml__vsdtypes, 0, GWEN_XML_FLAGS_DEFAULT);
  if (!tNode) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not parse internal derived types node");
    return -1;
  }

  if (GWEN_XSD_ImportSchema(e, tNode, "xsdtypes.xsd")) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Could not import internal derived types node");
    GWEN_XMLNode_free(tNode);
    return -1;
  }

  return 0;
}



int GWEN_XSD_ImportStart(GWEN_XSD_ENGINE *e) {
  assert(e);
  e->derivedTypesImported=0;
  return 0;
}


int GWEN_XSD_ImportEnd(GWEN_XSD_ENGINE *e) {
  int rv;

  assert(e);
  rv=GWEN_XSD__FinalizeNameSpaces(e);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  rv=GWEN_XSD__FinishXsdDoc(e);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_XSD_AddNameSpace(GWEN_XSD_ENGINE *e,
                          const char *prefix,
                          const char *name,
                          const char *url,
                          const char *localUrl){
  GWEN_XSD_NAMESPACE *ns;

  if (GWEN_XSD_NameSpace_List_GetCount(e->nameSpaces)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Schema import already started, "
              "please setup namespace before.");
    return -1;
  }

  ns=GWEN_XSD_NameSpace_List_First(e->userNameSpaces);
  while(ns) {
    if (strcasecmp(prefix, ns->id)==0)
      break;
    ns=GWEN_XSD_NameSpace_List_Next(ns);
  }
  if (ns) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "A namespace with prefix \"%s\" already exists",
              prefix);
  }

  ns=GWEN_XSD_NameSpace_new(prefix, name, url, localUrl);
  GWEN_XSD_NameSpace_List_Add(ns, e->userNameSpaces);

  return 0;
}







