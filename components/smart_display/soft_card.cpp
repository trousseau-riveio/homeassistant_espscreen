// Soft (local fork): its cards, laid out after the reference (theme::soft, settings_screen::theme 3).
//
// Compiled on its own, not inline in main.cpp: on Xtensa (the ESP32 and the ESP32-S3) an l32r instruction reaches at most
// 256 KB back to its literal, and every header of this component lands in main.cpp. With Soft's layout there the CYD
// failed to link ("dangerous relocation: l32r: literal target out of range"); its own file gives it its own literal pool,
// as page_receiver.cpp has (docs/RELEASING.md, "The Xtensa literal range"). runtime_tiles::render_slot calls it.
#include "runtime_tiles.h"
namespace runtime_tiles {
// Soft's player (local fork): the music, not the player. While a track plays or rests the card shows its cover, its
// title and its artist and the keys, never the player's icon or name; a player with nothing to play shows its name and
// its state beside a music note in the cover's place. Each shape of card has its own arrangement:
//  - a strip (more than twice as wide as tall): one row, the cover at the left, the words, the keys at the right;
//  - a landscape card: the cover as tall as the card at the left, the words over the keys beside it;
//  - a square card: the cover at the top left with the words beside it, the keys along the bottom;
//  - a standing card: the cover on top, the words under it, centred, the keys at the bottom;
//  - one cell: the cover with play beside it at the top, the words at the bottom.
// The volume (Controls: Volume, or Volume and playback where there is room for both) is Soft's own slider, over the
// keys. Keys keep a finger's size (ui::touch_min): a row that does not fit drops previous first, then next, play last.
// The tile's options say what it shows:
//  - Display: Cover puts the tile's picture in a rounded square, cut over the card's own colour (tile_ask); Standard
//    shows no square at all, the words and the keys take the card;
//  - Second line: Automatic is the artist (the album without one), None leaves the title alone, and a text or a value
//    of the player's says that instead;
//  - Controls: the keys, the volume, both or none;
//  - Icon: a chosen icon stands in the cover's place until the cover is here, the music note otherwise.
static void soft_media(Widgets &w,const Tile &t,const SoftCard &c){
  w.soft_drawn=true;w.soft_watch=false;w.soft_media=true;
  hide_extra(w);lv_obj_add_flag(w.progress,LV_OBJ_FLAG_HIDDEN);set_hidden(w.unit,true);
  const int pad=std::clamp(std::min(tile_width(w),tile_height(w))*9/100,ui::px(12),ui::px(24));
  for(auto prop:{LV_STYLE_PAD_TOP,LV_STYLE_PAD_BOTTOM,LV_STYLE_PAD_LEFT,LV_STYLE_PAD_RIGHT})set_number(w.tile,prop,pad);
  const int cw=content_width(w),ch=content_height(w);
  const int gap=std::max(ui::px(8),pad*2/3);
  const int finger=ui::touch_min();
  const auto &x=t.extra();
  const bool track=media_track(t);
  const std::string title=track&&!x.media_title.empty()?x.media_title:(t.name.empty()?t.entity:t.name);
  std::string artist=track?(x.media_artist.empty()?x.media_album:x.media_artist):c.value;
  std::string chosen;
  if(chosen_subtitle(t,chosen))artist=chosen;
  const bool art=t.display=="cover";
  // A note in the cover's place until the cover is here, and where there is none: never the player's own icon, unless
  // one was chosen for the tile.
  label(w.icon,t.icon.empty()?"\xF3\xB0\x8E\x87":t.icon.c_str());  // mdi:music-note
  const std::string kind=tile_controls::panel_kind(t);
  const bool live=t.available()&&!t.builtin()&&fresh();
  const bool volume_asked=kind=="volume"||tile_controls::media_volume_selected(t);
  const bool volume=volume_asked&&live&&(t.supported&tile_controls::feature::MEDIA_VOLUME_SET);
  const bool keys_asked=tile_controls::is_key_row(kind);
  // The shape of the card.
  const bool strip=cw*10>=ch*22;
  const bool narrow=cw<3*finger+2*ui::px(6);
  const bool small=!strip&&narrow&&ch*10<cw*17;
  const bool landscape=!strip&&!small&&cw*10>=ch*13;
  const bool standing=!strip&&!small&&!landscape&&(narrow||ch*10>=cw*13);
  // The keys: built as every card's (layout_panel, the keys alone), then sized and placed here.
  std::array<int,3> shown{};int count=0;
  if(keys_asked&&live&&layout_panel(w,t,c.large_tile,cw,ch,-1,0)){
    for(int n=0;n<3;++n)if(w.keys[n]&&!lv_obj_has_flag(w.keys[n],LV_OBJ_FLAG_HIDDEN))shown[count++]=n;
  }else hide_panel(w);
  int play=-1;
  for(int i=0;i<count;++i)if(w.key_commands[shown[i]]==tile_controls::MEDIA_PLAY_PAUSE)play=i;
  const int side=std::min(cw,ch);
  const int key=strip?std::clamp(ch*60/100,finger,ui::px(56)):small?finger:std::clamp(side*21/100,finger,ui::px(64));
  const int big=play>=0&&!strip&&!small?std::min(key*6/5,std::max(key,side/3)):key;
  const int key_gap=strip||small?ui::px(6):std::max(ui::px(8),key/3);
  // Which keys a row of `fit` shows (3: all, 2: play and next, 1: play), and how wide that row is.
  auto key_on=[&](int fit,int i){return fit>=3||i==play||(fit==2&&play>=0&&i>play);};
  auto row_width=[&](int fit){
    int width=0,laid=0;
    for(int i=0;i<count;++i)if(key_on(fit,i)){width+=i==play?big:key;++laid;}
    return width+std::max(0,laid-1)*key_gap;
  };
  // The most keys a row of `room` holds: previous goes first, then next.
  auto row_keys=[&](int room){
    if(row_width(3)<=room)return 3;
    if(play>=0&&row_width(2)<=room)return 2;
    if(play>=0&&row_width(1)<=room)return 1;
    return 0;
  };
  int keys_w=0;
  // Lays the row of `fit` keys from `left` (in the middle of `room` when centred), its middle line at `mid`; returns
  // its width.
  auto lay_keys=[&](int fit,int left,int mid,bool centred,int room){
    std::array<bool,3> on{};
    for(int i=0;i<count;++i)on[i]=key_on(fit,i);
    const int width=row_width(fit);
    int at=centred?left+std::max(0,(room-width)/2):left;
    int xs[3]={0,0,0},ws[3]={0,0,0};
    for(int i=0;i<count;++i){
      auto *k=w.keys[shown[i]];
      if(!on[i]){lv_obj_add_flag(k,LV_OBJ_FLAG_HIDDEN);w.key_commands[shown[i]]=tile_controls::NONE;continue;}
      const int s=i==play?big:key;xs[i]=at;ws[i]=s;at+=s+key_gap;
    }
    // The panel holds the row: as wide as it, as tall as its biggest key.
    const int top=mid-big/2;
    int first=-1;
    for(int i=0;i<count;++i)if(on[i]&&first<0)first=xs[i];
    if(first<0){hide_panel(w);return 0;}
    lv_obj_set_pos(w.panel,first,top);lv_obj_set_size(w.panel,width,big);
    for(int i=0;i<count;++i)if(on[i]){
      auto *k=w.keys[shown[i]];const int s=ws[i];
      lv_obj_set_size(k,s,s);lv_obj_set_pos(k,xs[i]-first,(big-s)/2);
      set_number(k,LV_STYLE_RADIUS,LV_RADIUS_CIRCLE);lv_obj_set_ext_click_area(k,std::max(0,(finger-s)/2));
      // A big key takes the card's icon face, as the full-page player's play key does.
      if(auto *icon=w.key_icons[shown[i]]){
        const lv_font_t *face=s>=ui::px(56)&&w.icon_font?w.icon_font:mini_icon_font?mini_icon_font:w.icon_font;
        if(lv_obj_get_style_text_font(icon,LV_PART_MAIN)!=face)set_font(icon,face);
        center_icon(icon);
      }
    }
    return width;
  };
  // The words: the title in the largest name face that its room calls for (two lines where `lines` allows and it needs
  // them), the artist a size smaller under it; what does not fit goes, the artist first. Returns the block's height.
  auto words=[&](int left,int top,int width,int height,int lines,bool centred,int target){
    int size=0;
    const auto names=soft_name_faces(),states=soft_state_faces();
    for(int i=2;i>0;--i)if(names[i]&&static_cast<int>(lv_font_get_line_height(names[i]))*4<=target*5){size=i;break;}
    auto letters=[&](const std::string &s,const lv_font_t *f,int i){return text_width(s,f)+(i==2?2:1)*static_cast<int>(s.size());};
    // One line that does not fit steps the face down before it is cut; two lines take what one does not hold.
    while(lines==1&&size>0&&names[size-1]&&letters(title,names[size],size)>width)--size;
    const lv_font_t *name_face=names[size]?names[size]:w.title_font;
    const int artist_size=std::max(0,size-1);
    const lv_font_t *artist_face=states[artist_size]?states[artist_size]:w.value_font;
    const int name_h=lv_font_get_line_height(name_face),artist_h=lv_font_get_line_height(artist_face);
    int title_lines=lines>1&&letters(title,name_face,size)>width?2:1;
    const int line_gap=std::max(ui::px(2),artist_h/5);
    bool with_artist=!artist.empty();
    auto block=[&]{return title_lines*name_h+(with_artist?line_gap+artist_h:0);};
    if(block()>height&&with_artist)with_artist=false;
    if(block()>height&&title_lines>1)title_lines=1;
    const int y=top+std::max(0,(height-block())/2);
    for(auto *o:{w.title,w.value})set_number(o,LV_STYLE_TEXT_LETTER_SPACE,size==2?2:1);
    set_font(w.title,name_face);set_font(w.value,artist_face);
    label(w.title,title);
    lv_obj_set_pos(w.title,left,y);lv_obj_set_size(w.title,std::max(1,width),title_lines*name_h);
    set_text_align(w.title,centred?LV_TEXT_ALIGN_CENTER:LV_TEXT_ALIGN_LEFT);
    set_hidden(w.value,!with_artist);
    if(with_artist){
      lv_obj_set_pos(w.value,left,y+title_lines*name_h+line_gap);lv_obj_set_size(w.value,std::max(1,width),artist_h);
      set_text_align(w.value,centred?LV_TEXT_ALIGN_CENTER:LV_TEXT_ALIGN_LEFT);
      fit_value(w.value,artist,"","",width);
    }
    return block();
  };
  label(w.value,artist);
  // Soft's slider for the volume, at x, y.
  auto volume_at=[&](int left,int top,int width,int height){
    lv_obj_align(w.slider,LV_ALIGN_TOP_LEFT,left,top);lv_obj_set_size(w.slider,width,height);
    lv_obj_set_ext_click_area(w.slider,slider_zone(w,height));slider_handle(w.slider,width,height);
    lv_obj_remove_flag(w.slider,LV_OBJ_FLAG_HIDDEN);travel_look(w.slider,t,false,true);
    if(!lv_obj_has_state(w.slider,LV_STATE_PRESSED)&&lv_slider_get_value(w.slider)!=slider_value(t))lv_slider_set_value(w.slider,slider_value(t),LV_ANIM_OFF);
  };
  lv_obj_add_flag(w.slider,LV_OBJ_FLAG_HIDDEN);
  const int slider_h=std::clamp(key*60/100,ui::px(22),ui::px(40));
  auto cover=[&](int s,int left,int top){
    if(!art||s<ui::px(28)){set_hidden(w.circle,true);if(w.picture)lv_obj_add_flag(w.picture,LV_OBJ_FLAG_HIDDEN);return;}
    soft_icon(w,t,s,top,left);set_number(w.circle,LV_STYLE_RADIUS,s/6);
  };
  if(strip){
    const int s=art?ch:0;
    cover(s,0,0);
    const int text_x=s>=ui::px(28)?s+gap:0;
    int right=cw,words_h=ch;
    if(volume&&!keys_asked){
      // The volume under the words where the strip is tall enough for both, else at the right of the row.
      const int under=std::clamp(ch*26/100,ui::px(18),ui::px(32));
      if(ch-under-gap/2>=ui::px(40)){volume_at(text_x,ch-under,cw-text_x,under);words_h=ch-under-gap/2;}
      else{
        const int sw=std::clamp(cw*34/100,ui::px(90),ui::px(220));
        volume_at(cw-sw,(ch-slider_h)/2,sw,slider_h);right=cw-sw-gap;
      }
    }else if(count){
      const int fit=row_keys(cw-text_x-ui::px(84)-gap);
      if(fit){keys_w=lay_keys(fit,0,ch/2,false,0);lv_obj_set_x(w.panel,cw-keys_w);right=cw-keys_w-gap;}
      else hide_panel(w);
    }
    words(text_x,0,std::max(1,right-text_x),words_h,1,false,words_h<ch?words_h/2:ch/3);
  }else if(small){
    const int s=art?std::max(0,std::min(ch*50/100,cw-(count&&play>=0?finger+gap:0))):0;
    cover(s,0,0);
    if(count&&play>=0){lay_keys(1,0,s/2,false,0);lv_obj_set_x(w.panel,cw-finger);}
    else hide_panel(w);
    const int top=std::max(s,count&&play>=0?finger:0)+gap/2;
    words(0,top,cw,ch-top,art?1:2,false,std::min(cw/7,(ch-top)/2));
  }else if(landscape){
    const int s=art?std::min(ch,cw*45/100):0;
    cover(s,0,(ch-s)/2);
    const int left=s?s+gap:0,room=cw-left;
    int bottom=ch;
    if(count){
      const int fit=row_keys(room);
      if(fit){lay_keys(fit,left,ch-big/2,true,room);bottom=ch-big-gap;}else hide_panel(w);
    }
    if(volume&&(!count||bottom-slider_h-gap>=ui::px(48))){volume_at(left,bottom-slider_h,room,slider_h);bottom-=slider_h+gap;}
    words(left,0,room,std::max(1,bottom),2,false,std::min(room/7,ch/5));
  }else if(standing){
    int bottom=ch;
    if(count){
      const int fit=row_keys(cw);
      if(fit){lay_keys(fit,0,ch-big/2,true,cw);bottom=ch-big-gap;}else hide_panel(w);
    }
    if(volume&&(!count||bottom-slider_h-gap>=ui::px(96))){volume_at(0,bottom-slider_h,cw,slider_h);bottom-=slider_h+gap;}
    // The words take what the cover leaves; the cover takes the width while the card is tall enough for both.
    const auto names=soft_name_faces();
    const int target=std::min(cw/7,ch/9);
    const int guess=2*std::max(ui::px(18),names[1]?static_cast<int>(lv_font_get_line_height(names[1])):ui::px(22))+ui::px(6);
    const int s=art?std::max(0,std::min(cw,bottom-gap-guess)):0;
    cover(s,(cw-s)/2,0);
    const int top=s>=ui::px(28)?s+gap:0;
    words(0,top,cw,std::max(1,bottom-top),art?2:3,true,art?target:std::min(cw/6,ch/7));
  }else{
    // A square card: the cover at the top left, the words beside it, the volume and the keys along the bottom.
    int bottom=ch;
    if(count){
      const int fit=row_keys(cw);
      if(fit){lay_keys(fit,0,ch-big/2,true,cw);bottom=ch-big-gap;}else hide_panel(w);
    }
      if(volume&&(!count||bottom-slider_h-gap>=ui::px(64))){volume_at(0,bottom-slider_h,cw,slider_h);bottom-=slider_h+gap;}
    const int s=art?std::max(0,std::min(cw*52/100,bottom-gap)):0;
    cover(s,0,0);
    const int left=s>=ui::px(28)?s+gap:0;
    // Without a cover the words take the room over the keys.
    if(!s)words(0,0,cw,std::max(1,bottom-gap),3,false,std::min(cw/6,std::max(1,bottom-gap)/3));
    else if(cw-left>=ui::px(80))words(left,0,cw-left,std::max(s,1),3,false,std::min((cw-left)/6,s/3));
    else words(0,s+gap/2,cw,std::max(1,bottom-s-gap/2),1,false,std::min(cw/7,ch/6));
  }
  (void)keys_w;
  soft_media_paint(w);
}
// The player's keys in Soft's colours: play filled in the palette's accent whether it plays or rests, the thing a
// finger looks for; previous and next bare, a circle under the finger only while pressed. Called after every layout
// and after the palette pass (style_panel), which paints every card's keys alike.
void soft_media_paint(Widgets &w){
  if(!w.soft_media||!w.panel)return;
  const auto &colors=theme::soft_colors();
  for(unsigned n=0;n<3;++n){
    auto *k=w.keys[n];if(!k)continue;
    if(w.key_commands[n]==tile_controls::MEDIA_PLAY_PAUSE){
      set_color(k,LV_STYLE_BG_COLOR,lv_color_hex(colors.knob));
      set_color(k,LV_STYLE_BG_COLOR,lv_color_hex(colors.knob),LV_STATE_CHECKED);
      set_color(k,LV_STYLE_BG_COLOR,lv_color_hex(theme::pressed(colors.knob)),LV_STATE_PRESSED);
      set_number(k,LV_STYLE_BG_OPA,LV_OPA_COVER);
      if(w.key_icons[n])set_color(w.key_icons[n],LV_STYLE_TEXT_COLOR,theme::color(theme::ON_ACCENT));
    }else{
      set_number(k,LV_STYLE_BG_OPA,LV_OPA_TRANSP);
      set_color(k,LV_STYLE_BG_COLOR,lv_color_hex(colors.circle),LV_STATE_PRESSED);
      if(w.key_icons[n])set_color(w.key_icons[n],LV_STYLE_TEXT_COLOR,lv_color_hex(colors.ink));
    }    }
}
void soft_card(Widgets &w,const Tile &t,const SoftCard &c){
  w.soft_media=false;
  if(c.d=="media_player"&&!w.full&&!c.watch&&!c.graph&&!t.favorite()){soft_media(w,t,c);return;}
  const std::string &d=c.d,&value=c.value,&unit=c.unit,&value_short=c.value_short,&value_tail=c.value_tail;
  const bool with_panel=c.with_panel,mini=c.mini,large_tile=c.large_tile,watch=c.watch,graph=c.graph;
  int content_w=c.content_w,content_h=c.content_h;
  int title_height=lv_font_get_line_height(lv_obj_get_style_text_font(w.title,LV_PART_MAIN));
  int value_height=lv_font_get_line_height(lv_obj_get_style_text_font(w.value,LV_PART_MAIN));
  if(t.stacked && !watch && !graph && !mini && !with_panel && !w.wide && !w.full){
    // Soft's stacked card ("layout": "stacked"): the icon in its circle over the name and the state, all centred, in
    // Soft's faces; as the look's own stacked card, no switch: a tap is the card's.
    w.soft_drawn=true;w.soft_watch=false;
    hide_panel(w);hide_extra(w);lv_obj_add_flag(w.progress,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(w.slider,LV_OBJ_FLAG_HIDDEN);set_hidden(w.value,true);
    const int pad=std::clamp(std::min(tile_width(w),tile_height(w))*9/100,ui::px(12),ui::px(24));
    for(auto prop:{LV_STYLE_PAD_TOP,LV_STYLE_PAD_BOTTOM,LV_STYLE_PAD_LEFT,LV_STYLE_PAD_RIGHT})set_number(w.tile,prop,pad);
    content_w=content_width(w);content_h=content_height(w);
    const int gap=std::max(ui::px(6),pad/2);
    const int target=std::min(content_w/8,content_h/6);
    int size=0;
    for(int i=2;i>0;--i)if(soft_name_faces()[i]&&static_cast<int>(lv_font_get_line_height(soft_name_faces()[i]))*4<=target*5){size=i;break;}
    const char *name_text=lv_label_get_text(w.title);
    while(size>0 && soft_name_faces()[size-1] && name_text &&
          text_width(name_text,soft_name_faces()[size])+(size==2?2:1)*static_cast<int>(strlen(name_text))>content_w)--size;
    const lv_font_t *name_face=soft_name_faces()[size]?soft_name_faces()[size]:w.title_font;
    const lv_font_t *word_face=soft_state_faces()[size]?soft_state_faces()[size]:w.value_font;
    const int spacing=size==2?2:1;
    for(auto *o:{w.title,w.unit})set_number(o,LV_STYLE_TEXT_LETTER_SPACE,spacing);
    set_font(w.title,name_face);set_font(w.unit,word_face);
    title_height=lv_font_get_line_height(name_face);const int word_h=lv_font_get_line_height(word_face);
    lv_obj_set_height(w.title,title_height);lv_obj_set_height(w.unit,word_h);
    const int lines=title_height+(value.empty()?0:ui::px(2)+word_h);
    const int circle=std::max(ui::px(24),std::min(std::clamp(std::min(content_w,content_h)*40/100,ui::px(36),ui::px(92)),content_h-gap-lines));
    const int top=std::max(0,(content_h-(circle+gap+lines))/2);
    soft_icon(w,t,circle,top,(content_w-circle)/2);
    set_text_align(w.title,LV_TEXT_ALIGN_CENTER);set_text_align(w.unit,LV_TEXT_ALIGN_CENTER);
    lv_obj_set_pos(w.title,0,top+circle+gap);lv_obj_set_width(w.title,content_w);
    set_hidden(w.unit,value.empty());
    if(!value.empty()){label(w.unit,value);lv_obj_set_pos(w.unit,0,top+circle+gap+title_height+ui::px(2));lv_obj_set_width(w.unit,content_w);}
  }
  else if(graph){
    // Soft's graph card (local fork): the head of every Soft card, the icon in its circle at the top left, the value at
    // the top right and the name under them, and the graph under the head to the card's border; on a card wider than
    // it is tall the head keeps the left and the graph takes the right, to the border on three sides (render_graph).
    w.soft_drawn=w.soft_watch=true;  // the value in ink, as on Soft's big value
    hide_panel(w);lv_obj_add_flag(w.progress,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(w.slider,LV_OBJ_FLAG_HIDDEN);set_hidden(w.unit,true);
    const int pad=std::clamp(std::min(tile_width(w),tile_height(w))*9/100,ui::px(12),ui::px(24));
    for(auto prop:{LV_STYLE_PAD_TOP,LV_STYLE_PAD_BOTTOM,LV_STYLE_PAD_LEFT,LV_STYLE_PAD_RIGHT})set_number(w.tile,prop,pad);
    content_w=content_width(w);content_h=content_height(w);
    const int gap=std::max(ui::px(8),pad*2/3);
    const bool beside=tile_width(w)*5>tile_height(w)*6;
    const int area_w=beside?content_w*55/100-gap:content_w;  // the head's room
    const int circle=std::clamp(std::min(area_w,content_h)*28/100,ui::px(34),ui::px(56));
    // The name's face from the head's width and the card's height, a size down for a name too long for its width.
    const int target=std::min(area_w/8,content_h/6);
    int size=0;
    for(int i=2;i>0;--i)if(soft_name_faces()[i]&&static_cast<int>(lv_font_get_line_height(soft_name_faces()[i]))*4<=target*5){size=i;break;}
    const char *name_text=lv_label_get_text(w.title);
    while(size>0 && soft_name_faces()[size-1] && name_text &&
          text_width(name_text,soft_name_faces()[size])+(size==2?2:1)*static_cast<int>(strlen(name_text))>area_w)--size;
    const lv_font_t *name_face=soft_name_faces()[size]?soft_name_faces()[size]:w.title_font;
    // The value a size up from the name's words where it fits beside the circle: it is what the card is read for.
    const int value_x=circle+gap,value_w=std::max(1,area_w-value_x);
    const lv_font_t *value_face=soft_state_faces()[size]?soft_state_faces()[size]:w.value_font;
    if(size<2 && soft_state_faces()[size+1] && text_width(value,soft_state_faces()[size+1])+static_cast<int>(value.size())<=value_w)
      value_face=soft_state_faces()[size+1];
    const int spacing=size==2?2:1;
    for(auto *o:{w.title,w.value})set_number(o,LV_STYLE_TEXT_LETTER_SPACE,spacing);
    set_font(w.title,name_face);set_font(w.value,value_face);
    title_height=lv_font_get_line_height(name_face);value_height=lv_font_get_line_height(value_face);
    lv_obj_set_height(w.title,title_height);lv_obj_set_height(w.value,value_height);
    set_text_align(w.title,LV_TEXT_ALIGN_LEFT);set_text_align(w.value,LV_TEXT_ALIGN_RIGHT);
    soft_icon(w,t,circle,0);
    lv_obj_set_pos(w.value,value_x,(circle-value_height)/2);lv_obj_set_width(w.value,value_w);
    fit_value(w.value,value,value_short,value_tail,value_w);set_hidden(w.value,false);
    const int name_y=circle+gap;
    lv_obj_set_pos(w.title,0,name_y);lv_obj_set_width(w.title,std::max(1,area_w));
    if(beside)render_graph(w,t,large_tile,area_w+gap,0,content_w-area_w-gap,content_h,graph_bleed(w,false,true,true,true));
    else{
      const int graph_y=name_y+title_height+gap;
      render_graph(w,t,large_tile,0,graph_y,content_w,std::max(8,content_h-graph_y),graph_bleed(w,true,false,true,true));
    }
  }
  else if(watch){
    // Soft's big value (local fork), three ways by the card's shape:
    //  - a card: the icon in its circle at the top left, the value large in the room under it, the name at the bottom;
    //  - a short wide card (twice as wide as tall): the circle and the name beside it on top, the value under them;
    //  - a strip (three times as wide as tall): one row, circle and name at the left, value at the right.
    // The unit follows the value on its baseline, smaller and grey. The value takes the largest face that fits its
    // room; where even the smallest does not fit with the unit, the unit goes, never a figure.
    w.soft_drawn=w.soft_watch=true;
    hide_panel(w);hide_extra(w);lv_obj_add_flag(w.progress,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(w.slider,LV_OBJ_FLAG_HIDDEN);
    const int pad=std::clamp(std::min(tile_width(w),tile_height(w))*9/100,ui::px(12),ui::px(24));
    for(auto prop:{LV_STYLE_PAD_TOP,LV_STYLE_PAD_BOTTOM,LV_STYLE_PAD_LEFT,LV_STYLE_PAD_RIGHT})set_number(w.tile,prop,pad);
    content_w=content_width(w);content_h=content_height(w);
    const int gap=std::max(ui::px(8),pad*2/3);
    const bool strip=content_w>=3*content_h, headed=!strip && content_w>=2*content_h;
    const int circle=strip?std::clamp(content_h*52/100,ui::px(34),ui::px(64)):
                     headed?std::clamp(content_h*34/100,ui::px(30),ui::px(48)):
                     std::clamp(std::min(content_w,content_h)*30/100,ui::px(34),ui::px(80));
    const char *name_text=lv_label_get_text(w.title);
    const int name_room=strip||headed?content_w-circle-gap:content_w;
    const int target=strip?content_h/4:headed?circle*3/4:std::min(content_w/8,content_h/6);
    int size=0;
    for(int i=2;i>0;--i)if(soft_name_faces()[i]&&static_cast<int>(lv_font_get_line_height(soft_name_faces()[i]))*4<=target*5){size=i;break;}
    while(size>0 && soft_name_faces()[size-1] && name_text &&
          text_width(name_text,soft_name_faces()[size])+(size==2?2:1)*static_cast<int>(strlen(name_text))>name_room*(strip?6:10)/10)--size;
    const lv_font_t *name_face=soft_name_faces()[size]?soft_name_faces()[size]:w.title_font;
    const lv_font_t *unit_face=soft_state_faces()[size]?soft_state_faces()[size]:w.value_font;
    const int spacing=size==2?2:1;
    set_number(w.title,LV_STYLE_TEXT_LETTER_SPACE,spacing);set_number(w.unit,LV_STYLE_TEXT_LETTER_SPACE,spacing);
    set_number(w.value,LV_STYLE_TEXT_LETTER_SPACE,0);
    set_font(w.title,name_face);title_height=lv_font_get_line_height(name_face);lv_obj_set_height(w.title,title_height);
    set_font(w.unit,unit_face);const int unit_h=lv_font_get_line_height(unit_face);lv_obj_set_height(w.unit,unit_h);
    set_text_align(w.title,LV_TEXT_ALIGN_LEFT);set_text_align(w.value,LV_TEXT_ALIGN_LEFT);set_text_align(w.unit,LV_TEXT_ALIGN_LEFT);
    // The value's room: under the circle and over the name; under the head; the right of a strip past its name.
    const int name_w=name_text?text_width(name_text,name_face)+spacing*static_cast<int>(strlen(name_text)):0;
    const int head=std::max(circle,title_height);
    const int room_x=strip?std::min(content_w*60/100,circle+gap+name_w+gap):0;
    const int room_w=content_w-room_x;
    const int room_h=strip?content_h:headed?content_h-head-gap/2:std::max(1,content_h-circle-gap-title_height-gap);
    bool with_unit=!unit.empty();
    int unit_w=with_unit?text_width(unit,unit_face)+spacing*static_cast<int>(unit.size()):0;
    const int unit_gap=ui::px(4);
    // The value's face: the largest of the figures that fits, or the name's for a word; the unit goes before a figure.
    const bool figures=soft_figures(value);
    const auto faces=figures?soft_value_faces():soft_name_faces();
    const lv_font_t *value_face=nullptr;
    for(int pass=0;pass<2 && !value_face;++pass){
      const int beside=with_unit?unit_gap+unit_w:0;
      for(int i=2;i>=0;--i){
        const lv_font_t *face=faces[i];
        if(face && static_cast<int>(lv_font_get_line_height(face))<=std::max(room_h,static_cast<int>(lv_font_get_line_height(faces[0]?faces[0]:w.value_font))) &&
           text_width(value,face)+beside<=room_w && (i==0 || static_cast<int>(lv_font_get_line_height(face))<=room_h)){value_face=face;break;}
      }
      if(!value_face && with_unit){with_unit=false;unit_w=0;}
    }
    if(!value_face)value_face=faces[0]?faces[0]:w.value_font;
    set_font(w.value,value_face);
    const int value_h=lv_font_get_line_height(value_face);lv_obj_set_height(w.value,value_h);
    const int value_w=text_width(value,value_face)+ui::px(2);
    const int value_x=strip?content_w-value_w-(with_unit?unit_gap+unit_w:0):0;
    const int value_y=strip?(content_h-value_h)/2:headed?head+gap/2+std::max(0,(room_h-value_h)/2):circle+gap+std::max(0,(room_h-value_h)/2);
    soft_icon(w,t,circle,strip?(content_h-circle)/2:headed?(head-circle)/2:0);
    lv_obj_set_pos(w.value,value_x,value_y);lv_obj_set_width(w.value,std::max(1,std::min(value_w,content_w-value_x)));
    label(w.value,value);set_hidden(w.value,false);
    set_hidden(w.unit,!with_unit);
    if(with_unit){
      const int baseline=value_y+value_h-value_face->base_line;
      label(w.unit,unit);lv_obj_set_width(w.unit,unit_w);
      lv_obj_set_pos(w.unit,value_x+value_w+unit_gap,baseline-(unit_h-unit_face->base_line));
    }
    // The name: beside the circle on a strip or a head, at the bottom of a card.
    const int text_x=strip||headed?circle+gap:0;
    const int text_y=strip?(content_h-title_height)/2:headed?(head-title_height)/2:content_h-title_height;
    lv_obj_set_pos(w.title,text_x,text_y);
    lv_obj_set_width(w.title,std::max(1,(strip?std::min(value_x,room_x)-gap:content_w)-text_x));
  }
  else{
    w.soft_watch=false;
    // Soft (local fork), every card alike after the reference: the icon in a circle at the top left, a word at the top
    // right, the name under them and the control at the bottom: a switch, or the slider the tile's Controls ask for
    // (standing at the right on a tall card). A strip, at least twice as wide as tall, is one row on its middle line:
    // circle, name, word, control. Everything is measured from the card: its padding, the circle and the control from
    // its shorter side, the words in three sizes from its width and height. Where the card is tall enough, the line
    // under the name (a lamp's brightness) keeps its room whether it shows or not, so the name stands still when it
    // comes; on a smaller card that line takes the word's place at the top right, the switch saying on or off itself.
    w.soft_drawn=true;
    hide_extra(w);lv_obj_add_flag(w.progress,LV_OBJ_FLAG_HIDDEN);
    const int pad=std::clamp(std::min(tile_width(w),tile_height(w))*9/100,ui::px(12),ui::px(24));
    for(auto prop:{LV_STYLE_PAD_TOP,LV_STYLE_PAD_BOTTOM,LV_STYLE_PAD_LEFT,LV_STYLE_PAD_RIGHT})set_number(w.tile,prop,pad);
    content_w=content_width(w);content_h=content_height(w);
    const int gap=std::max(ui::px(8),pad*2/3);
    const bool strip=content_w>=2*content_h;
    const std::string slide_kind=tile_controls::inline_kind(d);
    const bool live=t.available() && !t.builtin();
    const bool slider=live && !slide_kind.empty() && (tile_controls::panel_kind(t)==slide_kind || t.inline_control=="slider");
    const bool switched=live && !slider && soft_switched(t) && t.controls!="run";
    // The keys the tile's Controls ask for where Soft has none of its own: a cover's up, stop and down, a thermostat's
    // -/+ or modes, a player's keys, a list's arrows, a run key (layout_panel). They stand where Soft's switch stands, at
    // the bottom of a card or the right of a strip, on the cards the look gives them room on (with_panel).
    // A strip gives them at most half its width; a card too narrow for a row of three keys stands them in a column
    // at its bottom right, as a tall card stands its slider (a player's volume over its keys goes there: the keys alone).
    const std::string kind=tile_controls::panel_kind(t);
    const bool narrow=!strip && content_w<3*ui::touch_min()+2*ui::px(6);
    int panel_w=0,panel_h=0;
    bool stacked=false,panel_under=false;
    const int under_h=std::clamp(content_h*30/100,ui::px(24),ui::px(48));
    // A single card keeps its keys too (local fork): a blind's up, stop and down in a column at its right, as on a wall
    // switch; the look's other cards have no room for them on a single cell and open the card instead.
    const bool single_keys=!with_panel && !slider && !switched && tile_controls::is_key_row(kind) && !t.builtin() && !watch &&
                           fresh() && t.available() && t.column_span()==1 && t.row_span()==1 && !w.full;
    if((with_panel||single_keys) && !slider && !switched && layout_panel(w,t,large_tile,content_w,content_h,narrow?-1:0,strip?content_w*45/100:0)){
      panel_w=lv_obj_get_style_width(w.panel,LV_PART_MAIN);panel_h=lv_obj_get_style_height(w.panel,LV_PART_MAIN);
      if(panel_w>content_w && tile_controls::is_key_row(kind)){
        int shown=0,key_w=0,key_h=0,key_gap=ui::px(6);
        for(unsigned n=0;n<3;++n)if(w.keys[n] && !lv_obj_has_flag(w.keys[n],LV_OBJ_FLAG_HIDDEN)){
          key_w=lv_obj_get_style_width(w.keys[n],LV_PART_MAIN);key_h=lv_obj_get_style_height(w.keys[n],LV_PART_MAIN);++shown;
        }
        // Keys keep their size (ui::touch_min): a cover's column of three too tall for its card keeps open and close,
        // its stop being in the card's pop-up; any other row that does not fit stays off the card (below).
        if(shown==3 && 3*key_h+2*key_gap>content_h && d=="cover" && w.keys[1]){
          lv_obj_add_flag(w.keys[1],LV_OBJ_FLAG_HIDDEN);w.key_commands[1]=tile_controls::NONE;shown=2;
        }
        // What is left fits a row (a cover's open and close on a single card): a row at the bottom, as Soft's switch.
        const bool row=shown && shown*key_w+(shown-1)*key_gap<=content_w;
        for(unsigned n=0,i=0;n<3;++n)if(w.keys[n] && !lv_obj_has_flag(w.keys[n],LV_OBJ_FLAG_HIDDEN)){
          if(row)lv_obj_set_pos(w.keys[n],i*(key_w+key_gap),0);else lv_obj_set_pos(w.keys[n],0,i*(key_h+key_gap));++i;
        }
        if(shown && row){panel_w=shown*key_w+(shown-1)*key_gap;panel_h=key_h;lv_obj_set_size(w.panel,panel_w,panel_h);}
        else if(shown){panel_w=key_w;panel_h=shown*key_h+(shown-1)*key_gap;lv_obj_set_size(w.panel,panel_w,panel_h);stacked=true;}
      }
      // A slider of the look's panel (a humidifier's) runs under the row on a strip, as Soft's own does below.
      if(strip && w.control_slider && tile_controls::is_slider(kind) && kind!="volume" && content_h-under_h-gap>=ui::px(40)){
        lv_obj_set_pos(w.control_slider,0,0);lv_obj_set_size(w.control_slider,content_w,under_h);
        slider_handle(w.control_slider,content_w,under_h);
        panel_w=content_w;panel_h=under_h;lv_obj_set_size(w.panel,panel_w,panel_h);panel_under=true;
      }
      // Keys that fit the card in neither form stay off it: the card opens with a tap, as on the look's other cards.
      if(panel_w>content_w || panel_h>content_h){hide_panel(w);panel_w=panel_h=0;stacked=false;}
      // A -/+ pill: on a card too narrow for it in a row it stands, + over the number over -, at the card's bottom
      // right; a number wider than its room takes the card's smaller face.
      if(w.pill && w.pill_value && w.keys[0] && w.keys[1] && (kind=="setpoint"||kind=="stepper")){
        const int key=lv_obj_get_style_width(w.keys[0],LV_PART_MAIN),key_h=lv_obj_get_style_height(w.keys[0],LV_PART_MAIN);
        const lv_font_t *small=soft_state_faces()[1]?soft_state_faces()[1]:w.value_font;
        const lv_font_t *face=control_font?control_font:w.title_font;
        const char *number=lv_label_get_text(w.pill_value);
        if(narrow){
          lv_obj_set_size(w.pill,key,3*key_h);lv_obj_set_pos(w.keys[1],0,0);lv_obj_set_pos(w.keys[0],0,2*key_h);
          lv_obj_set_width(w.pill_value,key);lv_obj_set_x(w.pill_value,0);
          panel_w=key;panel_h=3*key_h;lv_obj_set_size(w.panel,panel_w,panel_h);stacked=true;
        }else{
          lv_obj_set_size(w.pill,panel_w,key_h);lv_obj_set_pos(w.keys[0],0,0);lv_obj_set_pos(w.keys[1],panel_w-key,0);
          lv_obj_set_width(w.pill_value,std::max(1,panel_w-2*key));lv_obj_set_x(w.pill_value,key);
        }
        const int room=lv_obj_get_style_width(w.pill_value,LV_PART_MAIN);
        if(number && text_width(number,face)+ui::px(6)>room)face=small;
        const int line=lv_font_get_line_height(face);
        if(lv_obj_get_style_text_font(w.pill_value,LV_PART_MAIN)!=face){set_font(w.pill_value,face);lv_obj_set_height(w.pill_value,line);}
        lv_obj_set_y(w.pill_value,narrow?key_h+(key_h-line)/2:(key_h-line)/2);
      }
    }else hide_panel(w);
    const bool standing=(slider && !strip && content_h>content_w*115/100) || stacked;
    // A standing slider on a card the whole page tall and half the glass wide or narrower fills the card's width under
    // a head of the circle and the name beside it, with no word at the top right; a shorter card keeps it at the right.
    lv_display_t *glass=lv_obj_get_display(w.tile);
    const bool column=standing && !stacked && glass && t.row_span()>=static_cast<int>(grid.rows) &&
                      tile_width(w)*2<=lv_display_get_horizontal_resolution(glass);
    const int head_circle=std::clamp(content_w*26/100,ui::px(30),ui::px(52));
    const int side=std::min(content_w,content_h);
    // The control: a switch half as tall as wide; a slider as tall as the switch, or a column on a tall card.
    int control_h=0,control_w=0;
    // A strip's slider runs under its icon and name, the whole width of the card, where the row above keeps room for
    // them; on a lower strip it stands at the right of the row.
    const bool under=strip && (slider || panel_under) && content_h-under_h-gap>=ui::px(40);
    if(under){control_h=under_h;control_w=content_w;}
    else if(strip){control_h=std::clamp(content_h*42/100,ui::px(28),ui::px(56));control_w=slider?std::clamp(content_w*36/100,ui::px(100),ui::px(240)):control_h*2;}
    else if(column){control_w=content_w;control_h=content_h;}
    else if(standing){control_w=std::clamp(content_w*24/100,ui::px(36),ui::px(64));control_h=content_h;}
    else{control_h=std::clamp(side*24/100,ui::px(26),ui::px(56));control_w=slider?content_w:control_h*2;}
    if(!slider&&!switched)control_w=control_h=0;
    if(panel_w){control_w=panel_w;control_h=panel_h;}
    // The words' room; on a strip with keys, what the circle and the keys leave of its row.
    const int head_h=under?content_h-control_h-gap:content_h;  // the row of the icon and the name
    int strip_circle=under?std::clamp(head_h*80/100,ui::px(30),ui::px(56)):std::clamp(content_h*(control_w?50:62)/100,ui::px(34),ui::px(72));
    // Keys that leave a strip's name less than a short word's room take the circle's place: the icon goes.
    if(strip && panel_w && !under && content_w-strip_circle-2*gap-control_w<ui::px(84))strip_circle=0;
    const int area_w=column?content_w-head_circle-gap:standing?content_w-control_w-gap:
                     under?std::max(1,content_w-strip_circle-gap):
                     strip&&control_w?std::max(1,content_w-(strip_circle?strip_circle+gap:0)-control_w-gap):content_w;
    // The words: the name from the card's width and height, a quarter of a strip's height.
    // Keys under the words take their height from the room the words are measured in.
    const int room_h=panel_w&&!strip&&!standing?std::max(1,content_h-control_h-gap):content_h;
    const int target=under?head_h/3:strip?content_h/4:column?area_w/5:std::min(area_w/(standing?7:8),room_h/6);
    int size=0;
    for(int i=2;i>0;--i)if(soft_name_faces()[i]&&static_cast<int>(lv_font_get_line_height(soft_name_faces()[i]))*4<=target*5){size=i;break;}
    // A name too long for its card's width steps down a size before it is cut.
    const char *name_text=lv_label_get_text(w.title);
    while((!strip || control_w) && size>0 && soft_name_faces()[size-1] && name_text &&
          text_width(name_text,soft_name_faces()[size])+(size==2?2:1)*static_cast<int>(strlen(name_text))>area_w)--size;
    const lv_font_t *name_face=soft_name_faces()[size]?soft_name_faces()[size]:w.title_font;
    const lv_font_t *word_face=soft_state_faces()[size]?soft_state_faces()[size]:w.value_font;
    const lv_font_t *second_face=soft_state_faces()[std::max(0,size-1)]?soft_state_faces()[std::max(0,size-1)]:word_face;
    const int spacing=size==2?2:1;
    for(auto *o:{w.title,w.value,w.unit})set_number(o,LV_STYLE_TEXT_LETTER_SPACE,spacing);
    set_font(w.title,name_face);set_font(w.value,second_face);set_font(w.unit,word_face);
    title_height=lv_font_get_line_height(name_face);value_height=lv_font_get_line_height(second_face);
    const int word_h=lv_font_get_line_height(word_face),line_gap=std::max(ui::px(2),value_height/6);
    lv_obj_set_height(w.value,value_height);lv_obj_set_height(w.unit,word_h);
    auto letters=[&](const std::string &text,const lv_font_t *face){return text_width(text,face)+spacing*static_cast<int>(text.size());};
    // A name too long for a narrow card beside its standing slider takes two lines (the label's dots end the second).
    // Still too long beside a standing slider: a name of several words takes two lines (the label's dots end the second).
    const bool two_lines=standing && name_text && strchr(name_text,' ') && letters(name_text,name_face)>area_w;
    const int name_h=two_lines?2*title_height:title_height;
    lv_obj_set_height(w.title,name_h);
    // The word: On or Off for what has a control, the state itself for the rest; the second line says the rest.
    const bool toggles=slider||switched;
    std::string word=toggles?std::string(tr(t.state=="on"?txt::ha_on:txt::ha_off)):value;
    std::string second=toggles && value!=word ? value : std::string();
    int circle=0,control_x=0,control_y=0,text_x=0,text_y=0,text_w=area_w,word_x=0,word_y=0,word_w=0,circle_y=0;
    bool second_line=true;
    if(strip){
      circle=strip_circle;circle_y=(head_h-circle)/2;
      control_x=under?0:content_w-control_w;control_y=under?content_h-control_h:(content_h-control_h)/2;
      // The control says on or off itself: a strip keeps its word for what has none (a lock, a sensor).
      if(toggles)word.clear();
      text_x=circle?circle+gap:0;
      const int row_end=control_w&&!under?control_x-gap:content_w;
      if(!word.empty()){
        const int need=letters(word,word_face);
        if(need+gap<=(row_end-text_x)/2){word_w=need;word_x=row_end-need;word_y=(head_h-word_h)/2;}
        else{second=second.empty()?word:word+" · "+second;word.clear();}
      }
      text_w=std::max(1,(word_w?word_x-gap:row_end)-text_x);
      text_y=(head_h-title_height-line_gap-value_height)/2;
    }else if(column){
      // The head: the circle, the name beside it on the circle's middle line with its second line under it.
      circle=head_circle;word.clear();
      text_x=circle+gap;text_w=area_w;
      const int block=name_h+(second.empty()?0:line_gap+value_height);
      text_y=std::max(0,(circle-block)/2);
      const int head=std::max(circle,block);
      control_x=0;control_y=head+gap;control_h=std::max(ui::px(40),content_h-control_y);
    }else{
      circle=std::clamp(std::min(area_w,room_h)*36/100,ui::px(36),ui::px(92));
      const int below=control_h&&!standing?control_h+gap:0;
      // Short of room for the second line, the circle gives way first, down to a size that still reads.
      const int roomy=content_h-gap-name_h-line_gap-value_height-below;
      if(circle>roomy && roomy>=ui::px(44))circle=roomy;
      // Still too short: the second line takes the word's place, the switch says on or off itself.
      if(circle+gap+name_h+line_gap+value_height+below>content_h){
        second_line=false;
        if(!second.empty()){word=second;second.clear();}
      }
      // And keys under the name that still leave it no room (a single card's open and close): a smaller circle.
      if(circle+gap+name_h+below>content_h)circle=std::max(ui::px(28),content_h-gap-name_h-below);
      word_x=circle+gap;word_w=std::max(1,area_w-word_x);word_y=std::max(0,(circle-word_h)/2);
      if(!word.empty() && letters(word,word_face)>word_w){
        if(second_line&&second.empty()){second=word;word.clear();}
        else word.clear();
      }
      const int block=name_h+(second_line?line_gap+value_height:0);
      // The name in the middle of the room between the circle and the control, or at the bottom with none.
      text_y=below||standing?circle+std::max(gap,(content_h-below-circle-block)/2):std::max(circle+gap,content_h-block);
      if(standing){control_x=content_w-control_w;control_y=stacked?content_h-control_h:0;}
      else if(control_w){control_x=0;control_y=content_h-control_h;}
    }
    if(circle)soft_icon(w,t,circle,circle_y);
    else{set_hidden(w.circle,true);if(w.picture)lv_obj_add_flag(w.picture,LV_OBJ_FLAG_HIDDEN);}
    set_text_align(w.title,LV_TEXT_ALIGN_LEFT);set_text_align(w.value,LV_TEXT_ALIGN_LEFT);set_text_align(w.unit,LV_TEXT_ALIGN_RIGHT);
    set_hidden(w.unit,word.empty());
    if(!word.empty()){label(w.unit,word);lv_obj_set_pos(w.unit,word_x,word_y);lv_obj_set_width(w.unit,word_w);}
    lv_obj_set_pos(w.title,text_x,text_y);lv_obj_set_width(w.title,text_w);
    set_hidden(w.value,second.empty());
    if(!second.empty()){lv_obj_set_pos(w.value,text_x,text_y+name_h+line_gap);lv_obj_set_width(w.value,text_w);fit_value(w.value,second,"","",text_w);}
    if(switched)soft_switch(w,t.state=="on",control_y,control_w,control_h,control_x);
    if(panel_w)lv_obj_set_pos(w.panel,control_x,control_y);
    if(slider){
      lv_obj_align(w.slider,LV_ALIGN_TOP_LEFT,control_x,control_y);
      lv_obj_set_size(w.slider,control_w,control_h);lv_obj_set_ext_click_area(w.slider,slider_zone(w,std::min(control_w,control_h)));
      if(standing)slider_handle_standing(w.slider,control_w,control_h);else slider_handle(w.slider,control_w,control_h);
      // A slider as wide as its card keeps the card's own corner rather than a pill's.
      if(column)for(auto part:{LV_PART_MAIN,LV_PART_INDICATOR})set_number(w.slider,LV_STYLE_RADIUS,ui::px(theme::SOFT_RADIUS)*2/3,part);
      lv_obj_remove_flag(w.slider,LV_OBJ_FLAG_HIDDEN);
      travel_look(w.slider,t,false,true);
      if(!lv_obj_has_state(w.slider,LV_STATE_PRESSED) && lv_slider_get_value(w.slider)!=slider_value(t))lv_slider_set_value(w.slider,slider_value(t),LV_ANIM_OFF);
    }else lv_obj_add_flag(w.slider,LV_OBJ_FLAG_HIDDEN);
  }
  (void)unit;(void)value_short;(void)value_tail;(void)mini;(void)title_height;(void)value_height;(void)d;
}
}  // namespace runtime_tiles
