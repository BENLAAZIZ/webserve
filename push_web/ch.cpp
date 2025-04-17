void Response::handleDelete(){
    struct stat st;

    if (stat(path.c_str(),  &st) < 0){
        err = "404";
        return;
    }
    if (!S_ISDIR(st.st_mode) && !S_ISREG(st.st_mode)){
        err = "404";
        return;
    }
    if (remove(path.c_str())){
        err = "403";
        return;
    }
    res += "204 ";
    res += statusCodes["204"];
    res += getDate();
    res += "\r\n";
}