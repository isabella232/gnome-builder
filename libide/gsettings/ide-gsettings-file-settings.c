/* ide-gsettings-file-settings.c
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

#include <glib/gi18n.h>

#include "ide-context.h"
#include "ide-debug.h"
#include "ide-file.h"
#include "ide-gsettings-file-settings.h"
#include "ide-language.h"
#include "ide-language-defaults.h"
#include "ide-settings.h"

struct _IdeGsettingsFileSettings
{
  IdeFileSettings  parent_instance;
  IdeSettings     *language_settings;
};

typedef struct
{
  const gchar             *key;
  const gchar             *property;
  GSettingsBindGetMapping  get_mapping;
} SettingsMapping;

G_DEFINE_TYPE (IdeGsettingsFileSettings, ide_gsettings_file_settings, IDE_TYPE_FILE_SETTINGS)

static gboolean
indent_style_get (GValue   *value,
                  GVariant *variant,
                  gpointer  user_data)
{
  if (g_variant_get_boolean (variant))
    g_value_set_enum (value, IDE_INDENT_STYLE_SPACES);
  else
    g_value_set_enum (value, IDE_INDENT_STYLE_TABS);
  return TRUE;
}

static SettingsMapping gLanguageMappings [] = {
  { "indent-width",                  "indent-width",             NULL             },
  { "insert-spaces-instead-of-tabs", "indent-style",             indent_style_get },
  { "right-margin-position",         "right-margin-position",    NULL             },
  { "show-right-margin",             "show-right-margin",        NULL             },
  { "tab-width",                     "tab-width",                NULL             },
  { "trim-trailing-whitespace",      "trim-trailing-whitespace", NULL             },
};

static void
ide_gsettings_file_settings_constructed (GObject *object)
{
  IdeGsettingsFileSettings *self = (IdeGsettingsFileSettings *)object;
  g_autofree gchar *relative_path = NULL;
  IdeLanguage *language;
  const gchar *lang_id;
  IdeContext *context;
  IdeFile *file;
  gsize i;

  IDE_ENTRY;

  G_OBJECT_CLASS (ide_gsettings_file_settings_parent_class)->constructed (object);

  if (!(file = ide_file_settings_get_file (IDE_FILE_SETTINGS (self))) ||
      !(language = ide_file_get_language (file)) ||
      !(lang_id = ide_language_get_id (language)))
    IDE_EXIT;

  context = ide_object_get_context (IDE_OBJECT (self));
  relative_path = g_strdup_printf ("/editor/language/%s/", lang_id);
  self->language_settings = ide_context_get_settings (context,
                                                      "org.gnome.builder.editor.language",
                                                      relative_path);

  for (i = 0; i < G_N_ELEMENTS (gLanguageMappings); i++)
    {
      SettingsMapping *mapping = &gLanguageMappings [i];

      ide_settings_bind_with_mapping (self->language_settings,
                                      mapping->key,
                                      self,
                                      mapping->property,
                                      G_SETTINGS_BIND_GET,
                                      mapping->get_mapping,
                                      NULL,
                                      NULL,
                                      NULL);
    }
}

static void
ide_gsettings_file_settings_finalize (GObject *object)
{
  IdeGsettingsFileSettings *self = (IdeGsettingsFileSettings *)object;

  g_clear_object (&self->language_settings);

  G_OBJECT_CLASS (ide_gsettings_file_settings_parent_class)->finalize (object);
}

static void
ide_gsettings_file_settings_class_init (IdeGsettingsFileSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = ide_gsettings_file_settings_constructed;
  object_class->finalize = ide_gsettings_file_settings_finalize;
}

static void
ide_gsettings_file_settings_init (IdeGsettingsFileSettings *self)
{
}
