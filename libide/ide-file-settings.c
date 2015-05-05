/* ide-file-settings.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "ide-file-settings"

#include <glib/gi18n.h>
#include <gtksourceview/gtksource.h>

#include "ide-enums.h"
#include "ide-file.h"
#include "ide-file-settings.h"

/*
 * WARNING: This file heavily uses XMACROS.
 *
 * XMACROS are not as difficult as you might imagine. It's basically just an inverstion
 * of macros. We have a defs file (in this case ide-file-settings.defs) which defines
 * information we need about properties. Then we define the macro called from that defs file
 * to do something we need, then include the .defs file.
 *
 * We do that over and over again until we have all the aspects of the object defined.
 */

typedef struct
{
  GPtrArray *children;

  IdeFile *file;

#define IDE_FILE_SETTINGS_PROPERTY(_1, name, field_type, _3, _pname, _4, _5, _6) \
  field_type name;
#include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(_1, name, field_type, _3, _pname, _4, _5, _6) \
  guint name##_set : 1;
#include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY
} IdeFileSettingsPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (IdeFileSettings, ide_file_settings, IDE_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_FILE,
#define IDE_FILE_SETTINGS_PROPERTY(NAME, _1, _2, _3, _pname, _4, _5, _6) \
  PROP_##NAME, \
  PROP_##NAME##_SET,
#include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY
  LAST_PROP
};

static GParamSpec *gParamSpecs [LAST_PROP];

#define IDE_FILE_SETTINGS_PROPERTY(_1, name, _2, ret_type, _pname, _3, _4, _5) \
ret_type ide_file_settings_get_##name (IdeFileSettings *self) \
{ \
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self); \
  gsize i; \
  g_return_val_if_fail (IDE_IS_FILE_SETTINGS (self), (ret_type)0); \
  if (priv->children != NULL) \
    { \
      for (i = 0; i < priv->children->len; i++) \
        { \
          IdeFileSettings *child = g_ptr_array_index (priv->children, i); \
          if (ide_file_settings_get_##name##_set (child)) \
            return ide_file_settings_get_##name (child); \
        } \
    } \
  return priv->name; \
}
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(_1, name, field_name, ret_type, _pname, _3, _4, _5) \
gboolean ide_file_settings_get_##name##_set (IdeFileSettings *self) \
{ \
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self); \
  g_return_val_if_fail (IDE_IS_FILE_SETTINGS (self), FALSE); \
  return priv->name##_set; \
}
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, ret_type, _pname, _3, assign_stmt, _4) \
void ide_file_settings_set_##name (IdeFileSettings *self, \
                                   ret_type         name) \
{ \
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self); \
  g_return_if_fail (IDE_IS_FILE_SETTINGS (self)); \
  assign_stmt \
  priv->name##_set = TRUE; \
  g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_##NAME]); \
  g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_##NAME##_SET]); \
}
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, _2, _pname, _3, _4, _5) \
void ide_file_settings_set_##name##_set (IdeFileSettings *self, \
                                         gboolean         name##_set) \
{ \
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self); \
  g_return_if_fail (IDE_IS_FILE_SETTINGS (self)); \
  priv->name##_set = !!name##_set; \
  g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_##NAME##_SET]); \
}
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

/**
 * ide_file_settings_get_file:
 * @self: An #IdeFileSettings.
 *
 * Retrieves the underlying file that @self refers to.
 *
 * This may be used by #IdeFileSettings implementations to discover additional
 * information about the settings. For example, a modeline parser might load
 * some portion of the file looking for modelines. An editorconfig
 * implementation might look for ".editorconfig" files.
 *
 * Returns: (transfer none): An #IdeFile.
 */
IdeFile *
ide_file_settings_get_file (IdeFileSettings *self)
{
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self);

  g_return_val_if_fail (IDE_IS_FILE_SETTINGS (self), NULL);

  return priv->file;
}

static void
ide_file_settings_set_file (IdeFileSettings *self,
                            IdeFile         *file)
{
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self);

  g_return_if_fail (IDE_IS_FILE_SETTINGS (self));
  g_return_if_fail (IDE_IS_FILE (file));

  if (priv->file != file)
    {
      if (ide_set_weak_pointer (&priv->file, file))
        g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_FILE]);
    }
}

static void
ide_file_settings_finalize (GObject *object)
{
  IdeFileSettings *self = (IdeFileSettings *)object;
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self);

  g_clear_pointer (&priv->children, g_ptr_array_unref);
  g_clear_pointer (&priv->encoding, g_free);
  ide_clear_weak_pointer (&priv->file);

  G_OBJECT_CLASS (ide_file_settings_parent_class)->finalize (object);
}

static void
ide_file_settings_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  IdeFileSettings *self = IDE_FILE_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, ide_file_settings_get_file (self));
      break;

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _2, _3, _4, _5, _6, value_type) \
    case PROP_##NAME: \
      g_value_set_##value_type (value, ide_file_settings_get_##name (self)); \
      break;
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, _2, _pname, _3, _4, _5) \
    case PROP_##NAME##_SET: \
      g_value_set_boolean (value, ide_file_settings_get_##name##_set (self)); \
      break;
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_file_settings_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  IdeFileSettings *self = IDE_FILE_SETTINGS (object);

  switch (prop_id)
    {
    case PROP_FILE:
      ide_file_settings_set_file (self, g_value_get_object (value));
      break;

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _2, _3, _4, _5, _6, value_type) \
    case PROP_##NAME: \
      ide_file_settings_set_##name (self, g_value_get_##value_type (value)); \
      break;
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, _2, _pname, _3, _4, _5) \
    case PROP_##NAME##_SET: \
      ide_file_settings_set_##name##_set (self, g_value_get_boolean (value)); \
      break;
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_file_settings_class_init (IdeFileSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_file_settings_finalize;
  object_class->get_property = ide_file_settings_get_property;
  object_class->set_property = ide_file_settings_set_property;

  gParamSpecs [PROP_FILE] =
    g_param_spec_object ("file",
                         _("File"),
                         _("The IdeFile the settings represent."),
                         IDE_TYPE_FILE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, _2, _pname, pspec, _4, _5) \
  gParamSpecs [PROP_##NAME] = pspec;
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

#define IDE_FILE_SETTINGS_PROPERTY(NAME, name, _1, _2, _pname, pspec, _4, _5) \
  gParamSpecs [PROP_##NAME##_SET] = \
    g_param_spec_boolean (_pname"-set", \
                          _pname"-set", \
                          "If IdeFileSettings:"_pname" is set.", \
                          FALSE, \
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
# include "ide-file-settings.defs"
#undef IDE_FILE_SETTINGS_PROPERTY

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
ide_file_settings_init (IdeFileSettings *self)
{
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self);

  priv->indent_style = IDE_INDENT_STYLE_SPACES;
  priv->indent_width = -1;
  priv->insert_trailing_newline = TRUE;
  priv->newline_type = GTK_SOURCE_NEWLINE_TYPE_LF;
  priv->right_margin_position = 80;
  priv->tab_width = 8;
  priv->trim_trailing_whitespace = TRUE;
}

static void
ide_file_settings_child_notify (IdeFileSettings *self,
                                GParamSpec      *pspec,
                                IdeFileSettings *child)
{
  g_assert (IDE_IS_FILE_SETTINGS (self));
  g_assert (pspec != NULL);
  g_assert (IDE_IS_FILE_SETTINGS (child));

  if (pspec->owner_type == IDE_TYPE_FILE_SETTINGS)
    g_object_notify_by_pspec (G_OBJECT (self), pspec);
}

void
_ide_file_settings_prepend (IdeFileSettings *self,
                            IdeFileSettings *child)
{
  IdeFileSettingsPrivate *priv = ide_file_settings_get_instance_private (self);

  g_return_if_fail (IDE_IS_FILE_SETTINGS (self));
  g_return_if_fail (IDE_IS_FILE_SETTINGS (child));

  g_signal_connect_object (child,
                           "notify",
                           G_CALLBACK (ide_file_settings_child_notify),
                           self,
                           G_CONNECT_SWAPPED);

  if (priv->children == NULL)
    priv->children = g_ptr_array_new_with_free_func (g_object_unref);

  g_ptr_array_insert (priv->children, 0, g_object_ref (child));
}

IdeFileSettings *
ide_file_settings_new (IdeFile *file)
{
  GIOExtensionPoint *extension_point;
  IdeFileSettings *ret;
  IdeContext *context;
  GList *list;

  g_return_val_if_fail (IDE_IS_FILE (file), NULL);

  context = ide_object_get_context (IDE_OBJECT (file));
  ret = g_object_new (IDE_TYPE_FILE_SETTINGS,
                      "context", context,
                      "file", file,
                      NULL);

  extension_point = g_io_extension_point_lookup (IDE_FILE_SETTINGS_EXTENSION_POINT);
  list = g_io_extension_point_get_extensions (extension_point);

  for (; list; list = list->next)
    {
      GIOExtension *extension = list->data;
      g_autoptr(IdeFileSettings) child = NULL;
      GType gtype;

      gtype = g_io_extension_get_type (extension);

      if (!g_type_is_a (gtype, IDE_TYPE_FILE_SETTINGS))
        {
          g_warning ("%s is not an IdeFileSettings", g_type_name (gtype));
          continue;
        }

      child = g_object_new (gtype,
                            "file", file,
                            "context", context,
                            NULL);

      _ide_file_settings_prepend (ret, child);
    }

  return ret;
}
