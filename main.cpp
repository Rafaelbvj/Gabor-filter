#include <iostream>
#include <complex>
#include "con-matrix.h"
#include "image.hpp"
#include <math.h>
#include <gtk/gtk.h>

using namespace std;

typedef long double NUM;

NUM thet = M_PI/2,phi=0, lambd = 3, gam = 0.4,desvio=30;

//Gabor function
complex <NUM> Gabor(NUM x, NUM y, NUM dev, NUM theta, NUM thi, NUM lambda, NUM gama)
{

    NUM parametric_x = x*cos(theta) + y*sin(theta);
    NUM parametric_y = -x*sin(theta) + y*cos(theta);
    NUM gauss_part = exp(  ( (pow (parametric_x, 2)) + pow (gama*parametric_y, 2) ) / (2*pow(dev,2))  );
    NUM real_factor = cos(2*M_PIf32* parametric_x/lambda + thi);
    NUM imag_factor = sin(2*M_PIf32* parametric_x/lambda + thi);
    complex <NUM> num (real_factor/gauss_part, imag_factor/gauss_part);
    return num;
}
//A kernel, matrix of gabor values
Matrix <NUM> generate_kernel (int sz, bool op, NUM desvio, NUM theta, NUM phi, NUM lambda, NUM gama)
{
    Matrix <NUM> m(sz,sz);
    int limit_y = sz/2;
    int limit_x = sz/2;
    for(int i=-limit_y, k =0; k<sz; i++,k++)
    {
        for(int e=-limit_x,j=0; j<sz; e++,j++)
        {
            complex <NUM> teste = Gabor(i,e,desvio,theta,phi,lambda,gama);
            if(op)
            {
                m.set_value_XY(k,j,teste.real());

            }
            else
            {
                m.set_value_XY(k,j,teste.imag());
            }
        }
    }
    return m;
}

GtkWidget *window;
GtkWidget *im;
GtkWidget *vbox_im,*vbox_param;
GtkWidget *grid_im;

GtkWidget *button_update;
GtkWidget *edit_t,*edit_th,*edit_de,*edit_g,*edit_l,*edit_ks;
GtkWidget *label,*label1,*label2,*label3,*label4,*label5,*label6;
char *filename = NULL;

void OpenFile(void *c)
{
    //FIle chooser settings
    GtkWidget * filechooser = gtk_file_chooser_dialog_new("Salvar",0,GTK_FILE_CHOOSER_ACTION_OPEN,"Abrir",GTK_RESPONSE_ACCEPT);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter,"*.bmp");
    gtk_file_filter_set_name(filter,"Bitmap");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser),filter);
    gtk_widget_show_all(filechooser);
    gint res = gtk_dialog_run(GTK_DIALOG(filechooser));
    //Response
    switch(res)
    {
    case GTK_RESPONSE_ACCEPT:
        gtk_widget_destroy(im);

        gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));

        im = gtk_image_new_from_file(filename);

        gtk_box_pack_start(GTK_BOX(vbox_im),im,TRUE,TRUE,0);

        gtk_widget_show_all(window);

        break;


    }
    gtk_widget_destroy(filechooser);


}
void Update()
{
    if(filename == NULL)
    {
        GtkWidget *msg = gtk_message_dialog_new(0,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Abra um arquivo!");
        g_signal_connect_swapped (msg, "response",G_CALLBACK (gtk_widget_destroy),msg);
        gtk_widget_show_all(msg);
        return;
    }
    thet = atol(gtk_entry_get_text(GTK_ENTRY(edit_t)))*M_PIf32/180.0f ;
    phi = atol(gtk_entry_get_text(GTK_ENTRY(edit_th)))*M_PIf32/180.0f;
    desvio = atol(gtk_entry_get_text(GTK_ENTRY(edit_de)))*M_PIf32/180.0f;
    gam = atol(gtk_entry_get_text(GTK_ENTRY(edit_g)))*M_PIf32/180.0f;
    lambd = atol(gtk_entry_get_text(GTK_ENTRY(edit_l)))*M_PIf32/180.0f;
    int size_kernel = atoi(gtk_entry_get_text(GTK_ENTRY(edit_ks)));

    Image ima(filename);
    if(size_kernel > ima.get_bitmap().biHeight || size_kernel > ima.get_bitmap().biWidth)
    {
        GtkWidget *msg = gtk_message_dialog_new(0,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Kernel maior que a imagem, escolha um menor.");
        g_signal_connect_swapped (msg, "response",G_CALLBACK (gtk_widget_destroy),msg);
        gtk_widget_show_all(msg);
        return;
    }

    Matrix <NUM> m = generate_kernel(size_kernel,true, desvio,thet,phi,lambd,gam);

    cout<<"Kernel:"<<endl<<m<<endl;

    Matrix <NUM> b,t,u;
    b.select_color(ima,RED_COLOR);
    u.select_color(ima,BLUE_COLOR);
    t.select_color(ima,GREEN_COLOR);

    unsigned char no_color[2];

    FILE *file = fopen("testeout.bmp","wb");
    M_BITMAP bmp = ima.get_bitmap();
    char id[70] = {'B','M'};
    fwrite(id,sizeof(char),2,file);
    fwrite(&bmp, sizeof(M_BITMAP),1,file);
    fwrite(&id[2],sizeof(char),bmp.bfOffBits-sizeof(bmp)-2,file);
    int npad;
    for(npad =0; ((bmp.biWidth)*3+npad) % 4 !=0 ; npad++) {}
    char *pad = new char[npad];
    Pixel ** p = ima.get_pixels();
    if(size_kernel == ima.get_bitmap().biHeight)
    {
        Matrix<NUM> *c = b<<m;
        Matrix<NUM> *d = t<<m;
        Matrix<NUM> *k = u<<m;
        for(size_t h=0; h<bmp.biHeight; h++)
        {
            for(size_t w=0; w<bmp.biWidth; w++)
            {

                p[h][w].R = (unsigned char) c->get_value_XY(w,h);
                p[h][w].B = (unsigned char) k->get_value_XY(w,h);;
                p[h][w].G = (unsigned char) d->get_value_XY(w,h);;

            }
            fwrite(p[h],sizeof(Pixel),bmp.biWidth,file);
            fwrite(pad,npad,1,file);
        }
        delete c;
    }
    else
    {

        for(size_t h=0; h<bmp.biHeight-size_kernel; h++)
        {
            for(size_t w=0; w<bmp.biWidth-size_kernel; w++)
            {
                Matrix <NUM> *c = (b.select_region(w+size_kernel,w,h,h+size_kernel));
                Matrix <NUM> *k = (t.select_region(w+size_kernel,w,h,h+size_kernel));
                Matrix <NUM> *x = (u.select_region(w+size_kernel,w,h,h+size_kernel));
                Matrix <NUM> *d = (*c)<<m;
                Matrix <NUM> *q = (*k)<<m;
                Matrix <NUM> *l = (*x)<<m;
                p[h][w].R = (unsigned char) d->sum_all_elements();
                p[h][w].B = (unsigned char) l->sum_all_elements();
                p[h][w].G = (unsigned char) q->sum_all_elements();
                delete c,k,x,d,q,l;

            }
            fwrite(p[h],sizeof(Pixel),bmp.biWidth,file);
            fwrite(pad,npad,1,file);

        }
    }


    fclose(file);
    gtk_widget_destroy(im);
    im = gtk_image_new_from_file("testeout.bmp");
    gtk_box_pack_start(GTK_BOX(vbox_im),im,TRUE,TRUE,0);
    gtk_widget_show_all(window);

}
int main(int argc,char *argv[])
{

    gtk_init(&argc,&argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Filtro de Gabor");
    gtk_window_set_default_size(GTK_WINDOW(window),500,500);

    button_update = gtk_button_new_with_label("Update");
    edit_t = gtk_entry_new();
    edit_th = gtk_entry_new();
    edit_de = gtk_entry_new();
    edit_g = gtk_entry_new();
    edit_l = gtk_entry_new();
    edit_ks = gtk_entry_new();

    gtk_entry_set_text(GTK_ENTRY(edit_t),"23");
    gtk_entry_set_text(GTK_ENTRY(edit_th),"0");
    gtk_entry_set_text(GTK_ENTRY(edit_de),"30");
    gtk_entry_set_text(GTK_ENTRY(edit_g),"0.4");
    gtk_entry_set_text(GTK_ENTRY(edit_l),"30");
    gtk_entry_set_text(GTK_ENTRY(edit_ks),"3");

    gtk_entry_set_input_purpose(GTK_ENTRY(edit_t),GTK_INPUT_PURPOSE_NUMBER);
    gtk_entry_set_input_purpose(GTK_ENTRY(edit_th),GTK_INPUT_PURPOSE_NUMBER);
    gtk_entry_set_input_purpose(GTK_ENTRY(edit_g),GTK_INPUT_PURPOSE_NUMBER);
    gtk_entry_set_input_purpose(GTK_ENTRY(edit_l),GTK_INPUT_PURPOSE_NUMBER);
    gtk_entry_set_input_purpose(GTK_ENTRY(edit_de),GTK_INPUT_PURPOSE_NUMBER);

    label1 = gtk_label_new("Theta (Graus):");
    label2 = gtk_label_new("Phi (Graus):");
    label3 = gtk_label_new("Desvio:");
    label4 = gtk_label_new("Gama:");
    label5 = gtk_label_new("Lambda:");
    label6 = gtk_label_new("Kernel (tamanho):");

    vbox_im = gtk_vbox_new(FALSE,0);
    vbox_param = gtk_vbox_new(FALSE,0);

    //Menu
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *menuitem_file = gtk_menu_item_new_with_label("Arquivo");
    GtkWidget *item_open = gtk_menu_item_new_with_label("Abrir");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem_file);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_file),menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_open);

    g_signal_connect(GTK_MENU_ITEM(item_open),"activate",G_CALLBACK(OpenFile),0);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    //******************//

    GtkWidget *align = gtk_grid_new();
    grid_im = gtk_grid_new();

    gtk_grid_set_column_spacing(GTK_GRID(align),1);

    gtk_box_pack_start(GTK_BOX(vbox_im),im,false,TRUE,0);

    gtk_box_pack_start(GTK_BOX(vbox_param), label1,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_t,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), label2,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_th,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), label3,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_de,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), label4,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_g,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), label5,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_l,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), label6,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), edit_ks,false,false,0);
    gtk_box_pack_start(GTK_BOX(vbox_param), button_update,false,false,0);

    gtk_grid_attach(GTK_GRID(grid_im), vbox_param,0,0,100,50);
    gtk_grid_attach_next_to(GTK_GRID(grid_im), vbox_im,vbox_param,GTK_POS_RIGHT,100,50);

    gtk_grid_attach(GTK_GRID(align), grid_im,0,3,100,50);

    gtk_container_add(GTK_CONTAINER(align),vbox);
    gtk_container_add(GTK_CONTAINER(window),align);


    g_signal_connect(GTK_BUTTON(button_update),"clicked",G_CALLBACK(Update),0);
    g_signal_connect(GTK_WINDOW(window),"destroy",G_CALLBACK(gtk_main_quit),0);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
