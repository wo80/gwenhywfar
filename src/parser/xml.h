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

#ifndef GWENHYWFAR_XML_H
#define GWENHYWFAR_XML_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/stringlist.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_XMLNODE_ALL XML Tree
 * @ingroup MOD_PARSER
 *
 */
/*@{*/

/** @defgroup MOD_XMLNODE XML Node
 *
 */
/*@{*/


/** @name Read Flags
 */
/*@{*/

/**
 * if set then comments are read. Otherwise they are ignored when reading
 * a file */
#define GWEN_XML_FLAGS_READ_COMMENTS  0x0001
/**
 * if set then toplevel elements are shared across all files (even included
 * ones, if the include tag/element appears in the top level)
 */
#define GWEN_XML_FLAGS_SHARE_TOPLEVEL      0x0002

/**
 * if set then the file given to the include tag/element are loaded to
 * the root of the XML tree regardless of the tag's location.
 */
#define GWEN_XML_FLAGS_INCLUDE_TO_TOPLEVEL 0x0004

/**
 * if set then include tags/elements are treated as any other tag
 * (i.e. no automatic file inclusion takes place. Instead the include
 * tag is stored like any other tag would be).
 */
#define GWEN_XML_FLAGS_IGNORE_INCLUDE      0x0008

/**
 * combination of other flags resembling the default flags
 */
#define GWEN_XML_FLAGS_DEFAULT 0
/*@}*/

/** The type of a property of a tag (in XML notation this would be
    called the attribute of an element). */
GWENHYWFAR_API
typedef struct GWEN__XMLPROPERTY GWEN_XMLPROPERTY;

/**
 * The possible types of a GWEN_XMLNODE.
 */
GWENHYWFAR_API
typedef enum {
  /** A node can be a tag (in XML notation these are called
      elements). */
  GWEN_XMLNodeTypeTag=0,
  /** A node can be some data. */
  GWEN_XMLNodeTypeData,
  /** A node can be some XML comment. */
  GWEN_XMLNodeTypeComment
} GWEN_XMLNODE_TYPE;

/** The abstract type XMLNODE. Each node is one node in the document
 * tree and can represent different things, see @ref
 * GWEN_XMLNODE_TYPE. */
GWENHYWFAR_API
typedef struct GWEN__XMLNODE GWEN_XMLNODE;


GWENHYWFAR_API
typedef int
    (*GWEN_XML_INCLUDE_FN)(GWEN_XMLNODE *n,
			   const char *path,
                           const char *file,
                           GWEN_STRINGLIST *sl,
			   GWEN_TYPE_UINT32 flags);


/** @name Constructors and Destructors
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_new(GWEN_XMLNODE_TYPE t, const char *data);

/**
 * Free the given node (including its children nodes)
 */
GWENHYWFAR_API
void GWEN_XMLNode_free(GWEN_XMLNODE *n);

/**
 * Free the given node and all nodes besides this one.
 * Hmm, this function should not be public, I think I will move it
 * to xml_p.h.
 */
GWENHYWFAR_API
void GWEN_XMLNode_freeAll(GWEN_XMLNODE *n);

/**
 * Create and return a deep copy of the given node.
 */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_dup(const GWEN_XMLNODE *n);
/*@}*/


/** @name Managing Properties/Attributes
 *
 * A property (in XML notation this is called attribute) is given
 * within a tag (in XML notation this is called element), like in this
 * example:
 *
 * @code
 * <tag property="1" />
 * @endcode
 */
/*@{*/
/**
 * Returns the value of the given property/attribute (or the default
 * value if the property/attribute does not exist or is empty).
 *
 * @param n node (must be a tag/element)
 * @param name name of the property/attribute
 * @param defaultValue default value to be returned if no value could
 * be retrieved
 */
GWENHYWFAR_API
  const char *GWEN_XMLNode_GetProperty(const GWEN_XMLNODE *n,
                                       const char *name,
                                       const char *defaultValue);

/**
 * Sets the value of a property/attribute. This property/attribute will be created if it does not
 * exist and overwritten if it does.
 * @param n node (must be a tag/element)
 * @param name name of the property/attribute
 * @param value new value of the property/attribute
 */
GWENHYWFAR_API
  void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n,
                                const char *name,
                                const char *value);

/**
 * This function copies the properties/attributes of one tag/element
 * to another one.
 *
 * @param tn destination node (must be a tag/element)
 * @param sn source node (must be a tag/element)
 * @param overwrite if !=0 then existing properties/attributes in the
 * destination node will be overwritten.
 */
GWENHYWFAR_API
  void GWEN_XMLNode_CopyProperties(GWEN_XMLNODE *tn,
                                   const GWEN_XMLNODE *sn,
                                   int overwrite);
/*@}*/

/** @name Type And Data
 *
 */
/*@{*/
/** Returns the type of the given node. */
GWENHYWFAR_API
GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(const GWEN_XMLNODE *n);

/** Returns the character data of the given node. */
GWENHYWFAR_API
const char *GWEN_XMLNode_GetData(const GWEN_XMLNODE *n);

/** Set the character data of the given node to the given value. This
 * function will create a deep copy of the character data. */
GWENHYWFAR_API
void GWEN_XMLNode_SetData(GWEN_XMLNODE *n, const char *data);
/*@}*/

/** @name Iterating Through an XML Tree
 *
 */
/*@{*/
/** INTERNAL. Iterates on the same level in the XML tree from the
 * given node to the next one on the same level (i.e. the returned
 * node has the same parent node as the given element). The returned
 * node may be a tag/element node, or a property/attribute node, or a
 * data node. You will probably prefer to use
 * GWEN_XMLNode_GetNextTag() instead of this function.
 *
 * @return The next node on the same level, or NULL if no more element
 * exists. */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_Next(const GWEN_XMLNODE *n);

/** INTERNAL. Descends in the XML tree to the first GWEN_XMLNODE below
 * the given node. The returned node may be a tag/element node, or a
 * property/attribute node, or a data node. You will probably prefer
 * to use GWEN_XMLNode_GetFirstTag() instead of this function.
 *
 * @return The first children tag/element, or NULL if none exists.
 */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetChild(const GWEN_XMLNODE *n);

/** Returns the parent node of the given node, or NULL if it already
 * is the root node. */
GWENHYWFAR_API
  GWEN_XMLNODE *GWEN_XMLNode_GetParent(const GWEN_XMLNODE *n);

/** Descends in the XML tree to the first children tag (in XML
 * notation they are called elements) below the given node.
 *
 * Different from GWEN_XMLNode_GetChild() this function only looks for
 * another tag/element and not for a (more general) node. You will
 * probably prefer this function instead of GWEN_XMLNode_GetChild().
 *
 * @return The first children tag/element, or NULL if none exists. */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetFirstTag(GWEN_XMLNODE *n);

/** Iterates on the same level in the XML tree from the given tag (in
 * XML notation they are called elements) to the next one on the same
 * level (i.e. the returned element has the same parent node as the
 * given element).
 *
 * Different from GWEN_XMLNode_Next() this function only looks for
 * another tag/element and not for a (more general) node. You will
 * probably prefer this function instead of GWEN_XMLNode_Next().
 *
 * @return The next tag/element on the same level, or NULL if no more
 * element exists. */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetNextTag(GWEN_XMLNODE *n);

/** Descends in the XML tree to the first children data node below the
 * given node. 
 *
 * Different from GWEN_XMLNode_GetChild() this function only looks for
 * another data node and not for a (more general) node. 
 *
 * @return The first children data node, or NULL if none exists. */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetFirstData(GWEN_XMLNODE *n);

/** Iterates on the same level in the XML tree from the given data
 * node to the next one on the same level (i.e. the returned element
 * has the same parent node as the given element). An XML element may
 * have multiple data nodes as children, and you use this function to
 * iterate through all of them.
 *
 * Different from GWEN_XMLNode_Next() this function only looks for
 * another data node  and not for a (more general) node.
 *
 * @return The next data node on the same level, or NULL if no more
 * data node exists. */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetNextData(GWEN_XMLNODE *n);

/**
 * Searches for the first matching tag/element below the given one.
 * Lets say you have the following XML file:
 * @code
 *  <DEVICES>
 *    <DEVICE id="dev1" />
 *    <DEVICE id="dev2" />
 *  </DEVICES>
 * @endcode
 * If you are looking for a device called "dev2" then you should call this
 * function like this:
 * @code
 *   tag=GWEN_XMLNode_FindFirstTag(root, "DEVICE", "id", "dev2");
 * @endcode
 * @return pointer to the tag/element if found, 0 otherwise
 * @param n tag/element below which to search
 * @param tname tag/element name (e.g. if the tag is "<TESTTAG>" then the
 * tag name is "TESTTAG"). Wildcards (like "*") are allowed.
 *
 * @param pname name of the property/attribute to check (if 0 then no
 * property/attribute comparison takes place). No wildcards allowed.
 *
 * @param pvalue optional value of the property/attribute to compare
 * against, wildcards allowed.
 */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_FindFirstTag(GWEN_XMLNODE *n,
                                        const char *tname,
                                        const char *pname,
                                        const char *pvalue);

/**
 * Searches for the next matching tag/element after the given one one
 * the same level (i.e. the returned element has the same parent node
 * as the given element).
 */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_FindNextTag(GWEN_XMLNODE *n,
                                       const char *tname,
                                       const char *pname,
                                       const char *pvalue);



/*@}*/


/** @name Managing Nodes
 *
 */
/*@{*/

/**
 * Adds a node as a child to another one. This function does not make deep
 * copies. Instead it takes over ownership of the new child.
 * @param n node to which the new node is to be added (i.e. the node which
 * becomes the parent of the second argument)
 * @param child child which is to be added (this function takes over ownership
 * of that node, so you MUST NOT free the node yourself)
 */
GWENHYWFAR_API
  void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);

/**
 * Unlinks the given child node from its parent without freeing it.
 * This function relinquishes the ownership of the child to the caller who
 * thereby becomes responsible for freeing this node.
 * @param n node which is suspected to be the parent of the second argument
 * @param child node which is to be unlinked
 */
GWENHYWFAR_API
  void GWEN_XMLNode_UnlinkChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);

/**
 * Unlinks and frees @b all children of the given node.
 */
GWENHYWFAR_API
  void GWEN_XMLNode_RemoveChildren(GWEN_XMLNODE *n);

/**
 * Adds the children of the second argument as new children to the first
 * one.
 * @param n node which is to become parent of the second argument's children
 * @param nn node whose children are to be moved.
 * @param copythem if 0 then the children will be moved (leaving the node of
 * the second argument without children), otherwise deep copies will be made
 * and the node from the second argument will not be altered.
 * co
 */
GWENHYWFAR_API
  void GWEN_XMLNode_AddChildrenOnly(GWEN_XMLNODE *n, GWEN_XMLNODE *nn,
                                    int copythem);

/**
 * This is a very primitive function. It looks for a node of the given type
 * and data matching the given one (case-insensitive) @b below the given node
 * (i.e. if a node is returned it will be a child of the given one).
 */
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_FindNode(GWEN_XMLNODE *n,
                                    GWEN_XMLNODE_TYPE t, const char *data);
/*@}*/


/** @name Reading And Writing From/To Streams
 *
 */
/*@{*/
/**
 * Reads exactly ONE tag/element (and all its subtags) from the given
 * bufferedIO.
 */
GWENHYWFAR_API
int GWEN_XML_Parse(GWEN_XMLNODE *n, GWEN_BUFFEREDIO *bio,
                   GWEN_TYPE_UINT32 flags);

/**
 * Reads all tags/elements from a file and adds them as children to
 * the given node.
 */
GWENHYWFAR_API
int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath,
                      GWEN_TYPE_UINT32 flags);

/**
 * Reads the given file. If the path is an absolute one it will be used
 * directly.
 * If it is a relative one the given search path will be searched in case the
 * file with the given name could not be loaded without a search path.
 * @param n XML node to store the read tags/elements in
 * @param filepath name (and optionally path) of the file to read
 * @param flags see @ref GWEN_XML_FLAGS_DEFAULT and others
 * @param searchPath a string list containing multiple multiple directories
 * to scan if the file could not be opened directly
 */
GWENHYWFAR_API
  int GWEN_XML_ReadFileSearch(GWEN_XMLNODE *n, const char *filepath,
                              GWEN_TYPE_UINT32 flags,
                              GWEN_STRINGLIST *searchPath);


/*@}*/


/** @name Debugging
 *
 */
/*@{*/

/**
 * Dumps the content of the given XML node and all its children.
 */
GWENHYWFAR_API
void GWEN_XMLNode_Dump(const GWEN_XMLNODE *n, FILE *f, int ind);
/*@}*/

/*@}*/ /* defgroup */


/** @defgroup MOD_XMLNODE_PATH XML Node Path
 *
 * This is used by the message engine module (@ref MOD_MSGENGINE_ALL).
 * A path consists of a list of nodes which are used while decoding/encoding
 * a message. A GWEN_XMLNODE_PATH serves as a LIFO stack (last-in-first-out).
 */
/*@{*/

GWENHYWFAR_API
  typedef struct GWEN_XMLNODE_PATH GWEN_XMLNODE_PATH;


GWENHYWFAR_API
GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new();
GWENHYWFAR_API
GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(const GWEN_XMLNODE_PATH *np);
GWENHYWFAR_API
void GWEN_XMLNode_Path_free(GWEN_XMLNODE_PATH *np);

/**
 * Adds a node to the path.
 */
GWENHYWFAR_API
int GWEN_XMLNode_Path_Dive(GWEN_XMLNODE_PATH *np,
                           GWEN_XMLNODE *n);

/**
 * Removes and returns the last added node (or 0 if that would bring us
 * beyond the root).
 */
GWENHYWFAR_API
  GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np);

/**
 * Dumps the contents of all XML nodes in the path.
 */
GWENHYWFAR_API
void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np);
/*@}*/ /* defgroup */
/*@}*/ /* defgroup (all)*/


#ifdef __cplusplus
}
#endif



#endif
