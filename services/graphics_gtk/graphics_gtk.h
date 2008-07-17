/**
 *  jdec graphics_gtk driver header.
 *
 *  @file graphics_gtk.h
 *  @author Jose Antonio Santos Cadenas <santoscadenas@gmail.com>
 *  @version 1.0
 *  @date 2007-12-4
 */

#ifndef __GRAPHICS_GTK_H__
#define __GRAPHICS_GTK_H__

#include <glade/glade-xml.h>

/**
 * Generic display refresh callback function's type definition
 * @return void
 */
typedef void (*guidisplay)(void);

/**
 * Type definition of the generic suscribe function to display callback
 * @param guidisplay The display callback fucntion implemented by the schema
 * @return 1 if the callback was registered correctly, othewise 0
 */
typedef int (*registerdisplay)(guidisplay f);

/**
 * Type definition of the generic suscribe function to display callback
 * @param guidisplay The display callback fucntion implemented by the schema
 * @return 1 if the callback was unregistered correctly, othewise 0
 */
typedef int (*deletedisplay)(guidisplay f);

/**
 * Search in the path the .glade file and loads it
 * @param file_name The name of the .glade file
 * @returns the newly created GladeXML object, or NULL on failure.
 */
typedef GladeXML* (*loadglade) (char * file_name);

#endif