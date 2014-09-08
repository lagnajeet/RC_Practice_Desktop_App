#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/fl_ask.H>

#ifdef WIN32
#include <FL/x.H>               // needed for fl_display
#include <windows.h>            // needed for LoadIcon()
#endif /*WIN32*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


using namespace std;

unsigned int count_sec;
unsigned int count_min;
Fl_Text_Display *clock1;
Fl_Input *p_no;
Fl_Help_View *p, *q, *info_box;
int pid;
int totalq;
int totalp;
int cur_qid;
int cur_ans;
int usr_ans;
int rs;
int enable_timer;
rapidxml::xml_node<> *questions,*node;
Fl_Button *But_A,*But_B,*But_C,*But_D,*But_E,*nxt_question;
void *obj_progress;

class cur_progress : public Fl_Box
{
    void draw()
    {
        Fl_Color c=FL_WHITE;
        Fl_Box::draw();
        fl_rectf(Fl_Box::x(),Fl_Box::y(),Fl_Box::w(),Fl_Box::h(),FL_BACKGROUND_COLOR);
        for(int i=0; i<num_circles; i++)
        {
            if(circle_data[i]==1)
                c=FL_RED;
            else if(circle_data[i]==2)
                c=FL_GREEN;
            else
                c=FL_WHITE;
            if(i==0)
            {
                fl_color(c);
                fl_pie(Fl_Box::x()+29*i,Fl_Box::y(), 29, 29, 0.0, 360.0);
                fl_color(FL_BLACK);
                fl_arc(Fl_Box::x()+29*i,Fl_Box::y(), 29, 29, 0.0, 360.0);
            }

            else
            {
                fl_color(c);
                fl_pie(Fl_Box::x()+33*i,Fl_Box::y(), 29, 29, 0.0, 360.0);
                fl_color(FL_BLACK);
                fl_arc(Fl_Box::x()+33*i,Fl_Box::y(), 29, 29, 0.0, 360.0);
            }
        }
        printf("\n");
    }

public:
    // CONSTRUCTOR
    cur_progress(int X,int Y,int W,int H,const char*L=0) : Fl_Box(X,Y,W,H,L)
    {
        num_circles=0;
        circle_data=NULL;
        box(FL_FLAT_BOX);
        color(FL_BACKGROUND_COLOR);
    }

    int num_circles;
    int* circle_data;
};

char* get_attr_value(rapidxml::xml_node<> *node, char* attr_name)
{
    for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
    {
        if(strcmp(attr_name, attr->name())==0)
            return attr->value();
    }
    return NULL;
}

static void Timer_CB(void *data)                // timer callback
{
    if(enable_timer==1)
    {
        Fl_Text_Buffer* buf;
        buf=new Fl_Text_Buffer();

        char text[1024];
        if(count_min<=0)
            sprintf(text,"Time elapsed : %d Secs",count_sec);
        else
        {
            if(count_min==1)
                sprintf(text,"Time elapsed : %d min , %d Secs",count_min,count_sec);
            else
                sprintf(text,"Time elapsed : %d mins , %d Secs",count_min,count_sec);
        }

        buf->text(text);
        count_sec=count_sec+1;
        if(count_sec>=60)
        {
            count_sec=0;
            count_min=count_min+1;
        }
        clock1->buffer(buf);
    }
    Fl::repeat_timeout(1, Timer_CB, data);
    //}
}
void disable_buttons(int d)
{
    if(d==1)
    {
        But_A->hide();
        But_B->hide();
        But_C->hide();
        But_D->hide();
        But_E->hide();
        info_box->show();
    }
    else
    {
        info_box->hide();
        But_A->show();
        But_B->show();
        But_C->show();
        But_D->show();
        But_E->show();
//        But_A->color(FL_BACKGROUND_COLOR);
//        But_B->color(FL_BACKGROUND_COLOR);
//        But_C->color(FL_BACKGROUND_COLOR);
//        But_D->color(FL_BACKGROUND_COLOR);
//        But_E->color(FL_BACKGROUND_COLOR);
    }
}
void check_answer(int usr_ans)
{
    rapidxml::xml_node<> *question_data=questions->first_node();
    int c=atoi(get_attr_value(question_data,"qid"));
    if(c<cur_qid)
    {
        while(c!=cur_qid)
        {
            question_data=question_data->next_sibling();
            c=atoi(get_attr_value(question_data,"qid"));
        }
    }
    else if(c>cur_qid)
    {
        while(c!=cur_qid)
        {
            question_data=question_data->previous_sibling();
            c=atoi(get_attr_value(question_data,"qid"));
        }
    }
    //at this point we are at the right question
    //printf("%s",get_attr_value(question_data,"qid"));
    rapidxml::xml_node<> *q_data=question_data->first_node();
    //printf("%s",get_attr_value(q_data,"txt"));
    Fl_Text_Buffer* buf;
    buf=new Fl_Text_Buffer();
    char options[6]= {'A','B','C','D','E'};
    char temp[16];
    buf->text("<p><font face='courier' size=5>");
    buf->append(get_attr_value(q_data,"txt"));
    buf->append("</font></p><table border='0' cellspacing='0' cellpadding='0'>");
    rapidxml::xml_node<> *answer=q_data->next_sibling();
    q_data=q_data->next_sibling();

    //printf("%d=",cur_qid);
    for(int i=0; i<5; i++)
    {

        if(i!=cur_ans)
        {
            q_data=q_data->next_sibling();
            if(i!=usr_ans)
            {
                buf->append("<tr><td width='20' valign='top'><font face='courier' size=5><b>");
                sprintf(temp,"%c",options[i]);
                buf->append(temp);
                buf->append(".</b></font></td><td width='365' valign='top'><font face='courier' size='5'>");
                buf->append(get_attr_value(q_data,"txt"));
                buf->append("</font></td></tr>");
            }
            else
            {
                buf->append("<tr><td width='20' valign='top' bgcolor='#ff6666'><font face='courier' size=5><b>");
                sprintf(temp,"%c",options[i]);
                buf->append(temp);
                buf->append(".</b></font></td><td width='365' valign='top' bgcolor='#ff6666'><font face='courier' size='5'>");
                buf->append(get_attr_value(q_data,"txt"));
                buf->append("</font></td></tr>");
            }

        }
        else
        {

            buf->append("<tr><td width='20' valign='top' bgcolor='#66ff66'><font face='courier' size=5><b>");
            sprintf(temp,"%c",options[i]);
            buf->append(temp);
            buf->append(".</b></font></td><td width='365' valign='top' bgcolor='#66ff66'><font face='courier' size='5'>");
            buf->append(get_attr_value(answer,"txt"));
            buf->append("</font></td></tr>");
        }

    }
    buf->append("</table>");

    cur_progress *o1 = (cur_progress*)obj_progress;
    Fl_Text_Buffer* buf1;
    buf1=new Fl_Text_Buffer();
    if(usr_ans==cur_ans)
    {
        buf1->text("<body><center><font face='courier' size='5' color='#009900'><b>&#10004;&nbsp;Correct answer</b></font></center></body>");
        o1->circle_data[cur_qid]=2;
    }

    else
    {
        buf1->text("<body><center><font face='courier' size='5' color='#cc3300'><b>&#10006;&nbsp;Incorrect. Correct answer is ");
        sprintf(temp,"%c",options[cur_ans]);
        buf1->append(temp);
        buf1->append("</b></font></center></body>");
        o1->circle_data[cur_qid]=1;
    }
    info_box->value(buf1->text());
    //o1->circle_data=cur_answers;    //1 = wrong, 2=right, 3=not asked
    o1->redraw();
    q->value(buf->text());
    disable_buttons(1);
    nxt_question->activate();
    nxt_question->take_focus();
    enable_timer=0;
}
void populate_question(int qid_no)
{
    rapidxml::xml_node<> *question_data=questions->first_node();
    int c=atoi(get_attr_value(question_data,"qid"));
    if(c<qid_no)
    {
        while(c!=qid_no)
        {
            question_data=question_data->next_sibling();
            c=atoi(get_attr_value(question_data,"qid"));
        }
    }
    else if(c>qid_no)
    {
        while(c!=qid_no)
        {
            question_data=question_data->previous_sibling();
            c=atoi(get_attr_value(question_data,"qid"));
        }
    }
    cur_qid=qid_no;

    //at this point we are at the right question
    //printf("%s",get_attr_value(question_data,"qid"));
    rapidxml::xml_node<> *q_data=question_data->first_node();
    //printf("%s",get_attr_value(q_data,"txt"));
    Fl_Text_Buffer* buf;
    buf=new Fl_Text_Buffer();
    char options[6]= {'A','B','C','D','E'};
    char temp[16];
    buf->text("<p><font face='courier' size=5>");
    buf->append(get_attr_value(q_data,"txt"));
    buf->append("</font></p><table border='0' cellspacing='0' cellpadding='0'>");
    rapidxml::xml_node<> *answer=q_data->next_sibling();
    q_data=q_data->next_sibling();
    srand (time(NULL));
    cur_ans=rand() % 5;
    //printf("%d  ",cur_ans);
    for(int i=0; i<5; i++)
    {

        if(i!=cur_ans)
        {
            q_data=q_data->next_sibling();
            buf->append("<tr><td width='20' valign='top'><font face='courier' size=5><b>");
            sprintf(temp,"%c",options[i]);
            buf->append(temp);
            buf->append(".</b></font></td><td width='365' valign='top'><font face='courier' size=5>");
            buf->append(get_attr_value(q_data,"txt"));
            buf->append("</font></td></tr>");
        }
        else
        {
            buf->append("<tr><td width='20' valign='top'><font face='courier' size=5><b>");
            sprintf(temp,"%c",options[i]);
            buf->append(temp);
            buf->append(".</b></font></td><td width='365' valign='top'><font face='courier' size=5>");
            buf->append(get_attr_value(answer,"txt"));
            buf->append("</font></td></tr>");
        }

    }
    buf->append("</table>");
    q->value(buf->text());
    disable_buttons(0);
    nxt_question->deactivate();
    enable_timer=1;
}
void gen_random(char *s, const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = '.';
    s[len+1] = 'p';
    s[len+2] = 'n';
    s[len+3] = 'g';
    s[len+4] = 0;
}
void populate_psg(int pid_no)
{
    rapidxml::xml_node<> *passage_data=node->first_node();
    char cur_img_file_name[16];
    int cur_pid=atoi(get_attr_value(passage_data,"pid"));
    if(cur_pid<pid_no)
    {
        while(cur_pid!=pid_no)
        {
            passage_data=passage_data->next_sibling();
            cur_pid=atoi(get_attr_value(passage_data,"pid"));
        }
    }
    else if(cur_pid>pid_no)
    {
        while(cur_pid!=pid_no)
        {
            passage_data=passage_data->previous_sibling();
            cur_pid=atoi(get_attr_value(passage_data,"pid"));
        }
    }
    pid=pid_no;
    //so at this point i should have the right pid
    char temp[4];
    sprintf(temp,"%d",(pid+1)); // as the pids go from 0 to n while we count from 1 to n+1
    p_no->value(temp);
    rapidxml::xml_node<> *p_data=passage_data->first_node();
    Fl_Text_Buffer* buf;
    buf=new Fl_Text_Buffer();
    buf->text(get_attr_value(p_data,"txt"));
    unsigned char *pixels;
    int w,h;

    gen_random(cur_img_file_name,10);
    pixels = stbi_load("line_no.gif", &w, &h, 0, 3);
    if (pixels == 0)
    {
        fprintf(stderr, "Couldn't open input file '%s'\n", "lines.png");
        exit(1);
    }
    //printf("%d",buf->length());
    stbi_write_png(cur_img_file_name, w, (int)(buf->length()/1.6), 3, pixels, w*3);

    buf->text("<body><table width='378' cellspacing=0 cellpadding=0 border=0><tr><td width='100' valign='top'><img src='");
    buf->append(cur_img_file_name);
    buf->append("'></td><td><font face='courier' size=5>");
    buf->append(get_attr_value(p_data,"txt"));
    buf->append("</font></p></td></tr></table><table>");
    p->value(buf->text());
    //free(questions);
    questions=p_data->next_sibling();
    rapidxml::xml_node<> *temp_node=questions->last_node();
    totalq=atoi(get_attr_value(temp_node,"qid"));
    cur_progress *o1 = (cur_progress*)obj_progress;
    free(o1->circle_data);
    o1->circle_data = (int*)malloc((totalq+1)*sizeof(int));
    for(int i=0;i<=totalq;i++)  //1 = wrong, 2=right, 3=not asked
        o1->circle_data[i]=3;
    o1->num_circles=totalq+1;
    o1->redraw();
    populate_question(0);
    count_sec=0;
    count_min=0;
    enable_timer=1;
    unlink(cur_img_file_name);
}
int total_psg()
{
    rapidxml::xml_node<> *passage_data=node->last_node();
    return atoi(get_attr_value(passage_data,"pid"));
}
void pid_no_input_CB(Fl_Widget*, void* userdata)
{
    //printf("works") ;
    int cur_pid=atoi(p_no->value());
    if(cur_pid<1)
        cur_pid=1;
    else if(cur_pid>totalp+1)
        cur_pid=totalp+1;
    populate_psg(cur_pid-1);
}

void Btn_Nxt_CB(Fl_Widget*, void* userdata)
{
    if(pid==totalp)
        pid=0;
    else
        pid=pid+1;
    populate_psg(pid);
}
void Btn_Prev_CB(Fl_Widget*, void* userdata)
{
    if(pid==0)
        pid=totalp;
    else
        pid=pid-1;
    populate_psg(pid);
}

void Btn_A(Fl_Widget*, void* userdata)
{
    usr_ans=0;
    check_answer(0);

}
void Btn_B(Fl_Widget*, void* userdata)
{
    usr_ans=1;
    check_answer(1);
}
void Btn_C(Fl_Widget*, void* userdata)
{
    usr_ans=2;
    check_answer(2);
}
void Btn_D(Fl_Widget*, void* userdata)
{
    usr_ans=3;
    check_answer(3);
}
void Btn_E(Fl_Widget*, void* userdata)
{
    usr_ans=4;
    check_answer(4);
}
void Btn_nxt_question(Fl_Widget*, void* userdata)
{
    if(cur_qid<totalq)
    {
        cur_qid=cur_qid+1;
        populate_question(cur_qid);
    }
    else
    {
        //srand (time(NULL));
        populate_psg((int)(rand() % totalp));
    }
}

void Btn_random_CB(Fl_Widget*, void* userdata)
{
    //srand (time(NULL));
    populate_psg((int)(rand() % totalp));

}
void MyDraw(const Fl_Label *o, int X, int Y, int W, int H, Fl_Align a) {
    fl_font(o->font, o->size);
    fl_color((Fl_Color)o->color);
    fl_draw(o->value, X, Y, W, H, a, o->image, 0);
}
// Global FLTK callback for measuring all labels
void MyMeasure(const Fl_Label *o, int &W, int &H) {
    fl_font(o->font, o->size);
    fl_measure(o->value, W, H, 0);
}
void Btn_about_CB(Fl_Widget*, void* userdata)
{
//    Fl_Text_Buffer* buf;
//    buf=new Fl_Text_Buffer();
//    buf->text("This program is releases under the GNU public license v 3.\nA copy of the license is included along with the program.\n\nFor any comment, suggestion, feature request or collaboration you can reach me at lagnajeet@gmail.com.\n\nThe developer takes no responsibility or claim for the data used with the software.");
    Fl::set_labeltype(FL_NORMAL_LABEL, MyDraw, MyMeasure);
    fl_message("\nThis program is releases under the GNU public license v 3.\nA copy of the license is included along with the program.\n\nYou can reach me at lagnajeet@gmail.com.\n\nThe developer takes no responsibility or claim for the data used with the software.");
}
int main (int argc, char ** argv)
{
    Fl_Double_Window *window;
    fl_register_images();
    //static Fl_GIF_Image image_img1 ("lines.gif");
    window = new Fl_Double_Window (850, 590,"RC Practice Utility v1.0");
    #ifdef WIN32
        window->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(101)));
    #endif /*WIN32*/
    Fl_Button *But_Nxt,*But_Prev, *But_random;
    p_no = new Fl_Input(90,5,50,30);
    But_A= new Fl_Button( 430, 510, 80, 30, "A" );
    But_A->labelsize(16);
    But_A->labelfont(FL_BOLD);
    But_A->callback(Btn_A);
    But_B= new Fl_Button( 512, 510, 80, 30, "B" );
    But_B->labelsize(16);
    But_B->labelfont(FL_BOLD);
    But_B->callback(Btn_B);
    But_C= new Fl_Button( 594, 510, 80, 30, "C" );
    But_C->labelsize(16);
    But_C->labelfont(FL_BOLD);
    But_C->callback(Btn_C);
    But_D= new Fl_Button( 676, 510, 80, 30, "D" );
    But_D->labelsize(16);
    But_D->labelfont(FL_BOLD);
    But_D->callback(Btn_D);
    But_E= new Fl_Button( 758, 510, 80, 30, "E" );
    But_E->labelsize(16);
    But_E->labelfont(FL_BOLD);
    But_E->callback(Btn_E);
    info_box = new Fl_Help_View(425,510,420,50,0);
    info_box->box(FL_FLAT_BOX);
    info_box->color(FL_BACKGROUND_COLOR);
    info_box->hide();
    info_box->scrollbar_size(1);
    nxt_question= new Fl_Button( 430, 550, 160, 30, "Next Question" );
    nxt_question->deactivate();
    nxt_question->callback(Btn_nxt_question,(void*)window);
    Fl_Button *about_btn= new Fl_Button( 676, 550, 160, 30, "About" );
    about_btn->callback(Btn_about_CB,(void*)window);
    But_Nxt= new Fl_Button( 150, 4, 80, 30, "Next" );
    But_Nxt->callback((Fl_Callback*)Btn_Nxt_CB, (void*)(0));
    But_random= new Fl_Button( 240, 4, 80, 30, "Random" );
    But_random->callback((Fl_Callback*)Btn_random_CB, (void*)(0));
    But_Prev= new Fl_Button( 4, 4, 80, 30, "Previous" );
    But_Prev->callback((Fl_Callback*)Btn_Prev_CB, (void*)(0));
    clock1 = new Fl_Text_Display(425,10,350,30,0);
    clock1->box(FL_NO_BOX);
    clock1->color(FL_BACKGROUND_COLOR);
    clock1->textsize(16);
    p_no->textsize(16);
    p_no->callback((Fl_Callback*)pid_no_input_CB, (void*)(0));
    p_no->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    q=new Fl_Help_View(425,40,420,460,0);
    q->box(FL_THIN_DOWN_BOX);
    p=new Fl_Help_View(3,40,423,500,0);
    p->box(FL_THIN_DOWN_BOX);
    FILE *fp;
    rapidxml::xml_document<> doc;
    pid=0;
    totalq=0;
    totalp=0;
    cur_qid=0;
    cur_ans=0;
    rs=0;
    usr_ans=-1;
    enable_timer=0;
    cur_progress p(5, 550,423, 30,0);
    obj_progress = (cur_progress*)window->child(15);

    rapidxml::file<> f("data.xml");
    doc.parse<0>(f.data());
    node = doc.first_node("passages");
    totalp=total_psg();
    //printf("%d",totalp);
    srand (time(NULL));
    populate_psg((int)(rand() % totalp));


    //printf("PID = %s\n",get_attr_value(pid,"val"));
    //printf("P_DATA = %s\n",get_attr_value(data,"txt"));



    enable_timer=1;
    Fl::add_timeout(1, Timer_CB);              // setup a timer
    window->end ();
    window->show (argc, argv);

    return(Fl::run());
    return 0;
}
