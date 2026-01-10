/***************************************************************************
 begin       : Tue Dec 30 2025
 copyright   : (C) 2025 by Martin Preuss
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

#include "./quicksort.h"


#define DISABLE_DEBUGLOG


static void _swap(void** a, void** b);
static void _quickSort(void* array[], int low, int high, GWEN_QUICKSORT_COMPARE_CB cb, void *arg);
static int _partition(void* array[], int low, int high, GWEN_QUICKSORT_COMPARE_CB cb, void *arg);



void GWEN_QuickSort(void* array[], int n, GWEN_QUICKSORT_COMPARE_CB cb, void *arg)
{
  _quickSort(array, 0, n-1, cb, arg);
}



// The QuickSort function implementation
void _quickSort(void* array[], int low, int high, GWEN_QUICKSORT_COMPARE_CB cb, void *arg) {
  if (low<high) {
    // pi is the partition return index of pivot
    int pi;

    pi=_partition(array, low, high, cb, arg);

    // recursion calls for smaller elements
    // and greater or equals elements
    _quickSort(array, low, pi-1, cb, arg);
    _quickSort(array, pi+1, high, cb, arg);
  }
}



// partition function
int _partition(void* array[], int low, int high, GWEN_QUICKSORT_COMPARE_CB cb, void *arg) {
  void *pivot;
  int i;
  int j;

  // Choose the pivot
  pivot=array[high];

  // Index of smaller element and indicates
  // the right position of pivot found so far
  i=low-1;

  // Traverse array[low..high] and move all smaller
  // elements to the left side. Elements from low to
  // i are smaller after every iteration
  for (j=low; j<=high-1; j++) {
    if (cb(array[j], pivot, arg)<0) { /* (array[j]<pivot) */
      i++;
      _swap(&array[i], &array[j]);
    }
  }

  // Move pivot after smaller elements and
  // return its position
  _swap(&array[i+1], &array[high]);
  return i+1;
}



void _swap(void** a, void** b) {
  void *t;

  t=*a;
  *a=*b;
  *b=t;
}






