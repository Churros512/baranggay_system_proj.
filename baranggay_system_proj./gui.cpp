#include "gui.h"
#include "database.h"
#include "document.h"
#include <gtk/gtk.h>
#include <cstring>
#include <string>
#include <vector>

static GtkWidget* resident_treeview = nullptr;
static GtkListStore* resident_store = nullptr;
static GtkWidget* document_treeview = nullptr;
static GtkListStore* document_store = nullptr;
static GtkWidget* population_label = nullptr;
static GtkWidget* export_status_label = nullptr;
static GtkWidget* firstname_entry = nullptr;
static GtkWidget* lastname_entry = nullptr;
static GtkWidget* age_entry = nullptr;
static GtkWidget* gender_entry = nullptr;
static GtkWidget* household_entry = nullptr;
static GtkWidget* voter_entry = nullptr;
static GtkWidget* search_entry = nullptr;
static GtkWidget* document_resident_entry = nullptr;
static GtkWidget* document_type_entry = nullptr;
static GtkWidget* clearance_resident_entry = nullptr;
static GtkWidget* clearance_purpose_entry = nullptr;

enum {
    COL_ID,
    COL_FIRSTNAME,
    COL_LASTNAME,
    COL_AGE,
    COL_GENDER,
    COL_HOUSEHOLD,
    COL_VOTER,
    NUM_RESIDENT_COLUMNS
};

enum {
    DOC_COL_ID,
    DOC_COL_RESIDENT,
    DOC_COL_TYPE,
    DOC_COL_STATUS,
    NUM_DOC_COLUMNS
};

static GtkWidget* create_resident_tree_view()
{
    resident_store = gtk_list_store_new(NUM_RESIDENT_COLUMNS,
                                       G_TYPE_INT,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING,
                                       G_TYPE_INT,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(resident_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);

    const char* headers[NUM_RESIDENT_COLUMNS] = {"ID", "First Name", "Last Name", "Age", "Gender", "Household", "Voter"};
    for (int i = 0; i < NUM_RESIDENT_COLUMNS; ++i) {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, headers[i], renderer, "text", i, NULL);
    }

    return view;
}

static GtkWidget* create_document_tree_view()
{
    document_store = gtk_list_store_new(NUM_DOC_COLUMNS,
                                       G_TYPE_INT,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING,
                                       G_TYPE_STRING);

    GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(document_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);

    const char* headers[NUM_DOC_COLUMNS] = {"ID", "Resident", "Document Type", "Status"};
    for (int i = 0; i < NUM_DOC_COLUMNS; ++i) {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view), -1, headers[i], renderer, "text", i, NULL);
    }

    return view;
}

static void refresh_resident_table(const std::vector<Resident>& residents)
{
    gtk_list_store_clear(resident_store);
    GtkTreeIter iter;

    for (const Resident& resident : residents) {
        gtk_list_store_append(resident_store, &iter);
        gtk_list_store_set(resident_store, &iter,
                           COL_ID, resident.id,
                           COL_FIRSTNAME, resident.firstname.c_str(),
                           COL_LASTNAME, resident.lastname.c_str(),
                           COL_AGE, resident.age,
                           COL_GENDER, resident.gender.c_str(),
                           COL_HOUSEHOLD, resident.household.c_str(),
                           COL_VOTER, resident.voter.c_str(),
                           -1);
    }
}

static void refresh_document_table(const std::vector<Document>& documents)
{
    gtk_list_store_clear(document_store);
    GtkTreeIter iter;

    for (const Document& request : documents) {
        gtk_list_store_append(document_store, &iter);
        gtk_list_store_set(document_store, &iter,
                           DOC_COL_ID, request.id,
                           DOC_COL_RESIDENT, request.resident.c_str(),
                           DOC_COL_TYPE, request.documentType.c_str(),
                           DOC_COL_STATUS, request.status.c_str(),
                           -1);
    }
}

static void update_summary()
{
    auto residents = getResidents();
    int male = 0;
    int female = 0;
    int other = 0;

    for (const Resident& resident : residents) {
        std::string gender = resident.gender;
        for (char& ch : gender) {
            ch = static_cast<char>(tolower(ch));
        }

        if (gender == "male" || gender == "m") {
            male++;
        } else if (gender == "female" || gender == "f") {
            female++;
        } else {
            other++;
        }
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Total Residents: %zu\nMale: %d\nFemale: %d\nOther: %d",
             residents.size(), male, female, other);
    gtk_label_set_text(GTK_LABEL(population_label), buffer);
}

static void on_add_resident(GtkButton* button, gpointer)
{
    Resident resident;
    resident.firstname = gtk_entry_get_text(GTK_ENTRY(firstname_entry));
    resident.lastname = gtk_entry_get_text(GTK_ENTRY(lastname_entry));
    resident.age = atoi(gtk_entry_get_text(GTK_ENTRY(age_entry)));
    resident.gender = gtk_entry_get_text(GTK_ENTRY(gender_entry));
    resident.household = gtk_entry_get_text(GTK_ENTRY(household_entry));
    resident.voter = gtk_entry_get_text(GTK_ENTRY(voter_entry));

    if (resident.firstname.empty() || resident.lastname.empty()) {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Name is required.");
        return;
    }

    if (addResident(resident)) {
        refresh_resident_table(getResidents());
        update_summary();
        gtk_entry_set_text(GTK_ENTRY(firstname_entry), "");
        gtk_entry_set_text(GTK_ENTRY(lastname_entry), "");
        gtk_entry_set_text(GTK_ENTRY(age_entry), "");
        gtk_entry_set_text(GTK_ENTRY(gender_entry), "");
        gtk_entry_set_text(GTK_ENTRY(household_entry), "");
        gtk_entry_set_text(GTK_ENTRY(voter_entry), "");
        gtk_label_set_text(GTK_LABEL(export_status_label), "Resident added.");
    } else {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Failed to add resident.");
    }
}

static void on_refresh_residents(GtkButton* button, gpointer)
{
    refresh_resident_table(getResidents());
    update_summary();
}

static void on_search_residents(GtkButton* button, gpointer)
{
    const char* text = gtk_entry_get_text(GTK_ENTRY(search_entry));
    if (text == nullptr || strlen(text) == 0) {
        refresh_resident_table(getResidents());
    } else {
        refresh_resident_table(searchResidents(text));
    }
}

static void on_add_document(GtkButton* button, gpointer)
{
    Document request;
    request.resident = gtk_entry_get_text(GTK_ENTRY(document_resident_entry));
    request.documentType = gtk_entry_get_text(GTK_ENTRY(document_type_entry));
    request.status = "PENDING";

    if (request.resident.empty() || request.documentType.empty()) {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Document request requires resident and type.");
        return;
    }

    if (addDocumentRequest(request)) {
        refresh_document_table(getDocuments());
        gtk_entry_set_text(GTK_ENTRY(document_resident_entry), "");
        gtk_entry_set_text(GTK_ENTRY(document_type_entry), "");
        gtk_label_set_text(GTK_LABEL(export_status_label), "Document request added.");
    } else {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Failed to add document request.");
    }
}

static void on_refresh_documents(GtkButton* button, gpointer)
{
    refresh_document_table(getDocuments());
}

static void on_generate_clearance(GtkButton* button, gpointer)
{
    const char* resident = gtk_entry_get_text(GTK_ENTRY(clearance_resident_entry));
    const char* purpose = gtk_entry_get_text(GTK_ENTRY(clearance_purpose_entry));

    if (resident == nullptr || purpose == nullptr || strlen(resident) == 0 || strlen(purpose) == 0) {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Resident and purpose are required.");
        return;
    }

    if (generateClearanceFor(resident, purpose)) {
        gtk_entry_set_text(GTK_ENTRY(clearance_resident_entry), "");
        gtk_entry_set_text(GTK_ENTRY(clearance_purpose_entry), "");
        gtk_label_set_text(GTK_LABEL(export_status_label), "Clearance generated.");
    } else {
        gtk_label_set_text(GTK_LABEL(export_status_label), "Failed to generate clearance.");
    }
}

static void on_export_csv(GtkButton* button, gpointer)
{
    if (exportResidentsCSV("exports/residents.csv")) {
        gtk_label_set_text(GTK_LABEL(export_status_label), "CSV exported: exports/residents.csv");
    } else {
        gtk_label_set_text(GTK_LABEL(export_status_label), "CSV export failed.");
    }
}

static GtkWidget* create_form_row(const char* label_text, GtkWidget** entry)
{
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* label = gtk_label_new(label_text);
    *entry = gtk_entry_new();
    gtk_widget_set_hexpand(*entry, TRUE);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), *entry, TRUE, TRUE, 0);
    return box;
}

void show_gui()
{
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Barangay System");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    GtkWidget* residents_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(residents_tab), 12);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("First Name:", &firstname_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("Last Name:", &lastname_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("Age:", &age_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("Gender:", &gender_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("Household:", &household_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), create_form_row("Voter (YES/NO):", &voter_entry), FALSE, FALSE, 0);

    GtkWidget* add_resident_button = gtk_button_new_with_label("Add Resident");
    g_signal_connect(add_resident_button, "clicked", G_CALLBACK(on_add_resident), NULL);
    gtk_box_pack_start(GTK_BOX(residents_tab), add_resident_button, FALSE, FALSE, 0);

    GtkWidget* search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search Residents:"), FALSE, FALSE, 0);
    search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(search_box), search_entry, TRUE, TRUE, 0);
    GtkWidget* search_button = gtk_button_new_with_label("Search");
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_residents), NULL);
    gtk_box_pack_start(GTK_BOX(search_box), search_button, FALSE, FALSE, 0);
    GtkWidget* refresh_residents_button = gtk_button_new_with_label("Refresh");
    g_signal_connect(refresh_residents_button, "clicked", G_CALLBACK(on_refresh_residents), NULL);
    gtk_box_pack_start(GTK_BOX(search_box), refresh_residents_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(residents_tab), search_box, FALSE, FALSE, 0);

    resident_treeview = create_resident_tree_view();
    GtkWidget* resident_scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(resident_scroller), resident_treeview);
    gtk_widget_set_vexpand(resident_scroller, TRUE);
    gtk_box_pack_start(GTK_BOX(residents_tab), resident_scroller, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), residents_tab, gtk_label_new("Residents"));

    GtkWidget* documents_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(documents_tab), 12);
    gtk_box_pack_start(GTK_BOX(documents_tab), create_form_row("Resident Name:", &document_resident_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(documents_tab), create_form_row("Document Type:", &document_type_entry), FALSE, FALSE, 0);
    GtkWidget* add_document_button = gtk_button_new_with_label("Add Request");
    g_signal_connect(add_document_button, "clicked", G_CALLBACK(on_add_document), NULL);
    gtk_box_pack_start(GTK_BOX(documents_tab), add_document_button, FALSE, FALSE, 0);
    GtkWidget* refresh_documents_button = gtk_button_new_with_label("Refresh Documents");
    g_signal_connect(refresh_documents_button, "clicked", G_CALLBACK(on_refresh_documents), NULL);
    gtk_box_pack_start(GTK_BOX(documents_tab), refresh_documents_button, FALSE, FALSE, 0);
    document_treeview = create_document_tree_view();
    GtkWidget* document_scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(document_scroller), document_treeview);
    gtk_widget_set_vexpand(document_scroller, TRUE);
    gtk_box_pack_start(GTK_BOX(documents_tab), document_scroller, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), documents_tab, gtk_label_new("Documents"));

    GtkWidget* reports_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(reports_tab), 12);
    population_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(population_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(reports_tab), population_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(reports_tab), create_form_row("Resident Name:", &clearance_resident_entry), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(reports_tab), create_form_row("Purpose:", &clearance_purpose_entry), FALSE, FALSE, 0);
    GtkWidget* generate_clearance_button = gtk_button_new_with_label("Generate Clearance");
    g_signal_connect(generate_clearance_button, "clicked", G_CALLBACK(on_generate_clearance), NULL);
    gtk_box_pack_start(GTK_BOX(reports_tab), generate_clearance_button, FALSE, FALSE, 0);
    GtkWidget* export_csv_button = gtk_button_new_with_label("Export Residents CSV");
    g_signal_connect(export_csv_button, "clicked", G_CALLBACK(on_export_csv), NULL);
    gtk_box_pack_start(GTK_BOX(reports_tab), export_csv_button, FALSE, FALSE, 0);
    export_status_label = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(export_status_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(reports_tab), export_status_label, FALSE, FALSE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), reports_tab, gtk_label_new("Reports"));

    refresh_resident_table(getResidents());
    refresh_document_table(getDocuments());
    update_summary();
    gtk_widget_show_all(window);
}
