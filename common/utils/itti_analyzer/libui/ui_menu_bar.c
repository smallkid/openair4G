#include <gtk/gtk.h>

#include "rc.h"
#include "ui_main_screen.h"
#include "ui_menu_bar.h"
#include "ui_callbacks.h"

int ui_menu_bar_create(GtkWidget *vbox)
{
    GtkWidget *menubar;
    GtkWidget *filemenu, *helpmenu;
    GtkWidget *file;
    GtkWidget *help;
    GtkWidget *quit;
    GtkWidget *about;

    if (!vbox)
        return RC_BAD_PARAM;

    menubar = gtk_menu_bar_new();

    /* Create the File submenu */
    filemenu = gtk_menu_new();

    file = gtk_menu_item_new_with_label("File");
    quit  = gtk_menu_item_new_with_label("Quit");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

    /* Create the Help submenu */
    helpmenu = gtk_menu_new();

    help = gtk_menu_item_new_with_label("Help");
    about = gtk_menu_item_new_with_label("About");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);

    /* Add the menubar to the vbox */
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 3);

    g_signal_connect(G_OBJECT(quit), "activate",
                     G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(G_OBJECT(about), "activate",
                     G_CALLBACK(ui_callback_on_about), NULL);

    return RC_OK;
}

int ui_toolbar_create(GtkWidget *vbox)
{
    GtkWidget *toolbar;
    GtkWidget *hbox;
    GtkWidget *iplabel;
    GtkWidget *portlabel;

    if (!vbox)
        return RC_BAD_PARAM;

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);

    /* Button to clear signal list and clear signal dissect view */
    {
        ui_main_data.signals_clear_button = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.signals_clear_button),
                                       "Start a new acquisition or replay");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.signals_clear_button, -1);

        gtk_widget_set_sensitive(GTK_WIDGET(ui_main_data.signals_clear_button), FALSE);

        g_signal_connect(G_OBJECT(ui_main_data.signals_clear_button), "clicked",
                        G_CALLBACK(ui_callback_signal_clear_list), NULL);
    }

    /* Button to open replay file */
    {
        ui_main_data.open_replay_file = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.open_replay_file),
                                    "Open new replay file");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.open_replay_file, -1);

        g_signal_connect(G_OBJECT(ui_main_data.open_replay_file), "clicked",
                        G_CALLBACK(ui_callback_on_open), NULL);
    }

    /* Button to go given signal number */
    {
        ui_main_data.signals_go_to_button = gtk_tool_button_new_from_stock(GTK_STOCK_INDEX);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.signals_go_to_button),
                                    "Goto signal");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.signals_go_to_button, -1);

        gtk_widget_set_sensitive(GTK_WIDGET(ui_main_data.signals_go_to_button), FALSE);

        g_signal_connect(G_OBJECT(ui_main_data.signals_go_to_button), "clicked",
                        G_CALLBACK(ui_callback_signal_go_to), NULL);
    }

    /* Button to go to first signal in list */
    {
        ui_main_data.signals_go_to_first_button = gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_FIRST);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.signals_go_to_first_button),
                                    "Goto first signal");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.signals_go_to_first_button, -1);

        gtk_widget_set_sensitive(GTK_WIDGET(ui_main_data.signals_go_to_first_button), FALSE);

        g_signal_connect(G_OBJECT(ui_main_data.signals_go_to_first_button), "clicked",
                        G_CALLBACK(ui_callback_signal_go_to_first), NULL);
    }

    /* Button to go to last signal in list */
    {
        ui_main_data.signals_go_to_last_button = gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_LAST);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.signals_go_to_last_button),
                                    "Goto last signal");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.signals_go_to_last_button, -1);

        gtk_widget_set_sensitive(GTK_WIDGET(ui_main_data.signals_go_to_last_button), FALSE);

        g_signal_connect(G_OBJECT(ui_main_data.signals_go_to_last_button), "clicked",
                        G_CALLBACK(ui_callback_signal_go_to_last), NULL);
    }

    /* Button to connect to remote */
    {
        ui_main_data.connect = gtk_tool_button_new_from_stock(GTK_STOCK_CONNECT);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.connect),
                                    "Connect to remote host");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.connect, -1);

        g_signal_connect(G_OBJECT(ui_main_data.connect), "clicked",
                        G_CALLBACK(ui_callback_on_connect), NULL);
    }

    /* Button to disconnect from remote */
    {
        ui_main_data.disconnect = gtk_tool_button_new_from_stock(GTK_STOCK_DISCONNECT);
        /* Set the tooltip text */
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(ui_main_data.disconnect),
                                    "Disconnect from remote host");
        gtk_toolbar_insert(GTK_TOOLBAR(toolbar), ui_main_data.disconnect, -1);

        /* Disabled at startup. Will be activated when a connection is established */
        gtk_widget_set_sensitive(GTK_WIDGET(ui_main_data.disconnect), FALSE);

        g_signal_connect(G_OBJECT(ui_main_data.disconnect), "clicked",
                        G_CALLBACK(ui_callback_on_disconnect), NULL);
    }

    /* No overflow menu */
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar), FALSE);

    iplabel = gtk_label_new("ip:");
    portlabel = gtk_label_new("port:");

    ui_main_data.ipentry = gtk_entry_new();
    /* Width of 15 characters for port number (ipv4 address) */
    gtk_entry_set_width_chars(GTK_ENTRY(ui_main_data.ipentry), 15);
    gtk_entry_set_text(GTK_ENTRY(ui_main_data.ipentry), "127.0.0.1");

    ui_main_data.portentry = gtk_entry_new();
    /* Width of 5 characters for port number (uint16_t) */
    gtk_entry_set_width_chars(GTK_ENTRY(ui_main_data.portentry), 5);
    gtk_entry_set_text(GTK_ENTRY(ui_main_data.portentry), "10007");

    gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), iplabel, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), ui_main_data.ipentry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), portlabel, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), ui_main_data.portentry, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    return RC_OK;
}