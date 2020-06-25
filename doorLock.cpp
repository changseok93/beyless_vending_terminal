//
// Created by changseok on 20. 6. 12..
//

#include "doorLock.h"

//default initializer (deprecated)
doorLock::doorLock(){
    log.print_log("default initiailizer called, this initializer used for debugging. please use another one!");

}

//door lock class initializer, gets GPIO pin number of lock, door sensors and door lock trigger
doorLock::doorLock(int lock, int door, int trigger){
    log.print_log("door lock initialize ... ");
    this->door_num = door;
    this->lock_num = lock;
    this->trigger_num = trigger;

    // create pin
    std::fstream pin_ctr;
    pin_ctr.open("/sys/class/gpio/export", std::ios::out);
    if (!pin_ctr.is_open()){
        log.print_log("\ncan not open file \"/sys/class/gpio/export\" please check again");
        std::exit(1);
    }

    pin_ctr << this->trigger_num;
    pin_ctr.seekg(0);
    pin_ctr << this->door_num;
    pin_ctr.seekg(0);
    pin_ctr << this->lock_num;
    pin_ctr.seekg(0);
    pin_ctr.close();

    // set direction
    char query[50];
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->trigger_num);
    pin_ctr.open(query);
    pin_ctr << "out";
    pin_ctr.close();

    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->door_num);
    pin_ctr.open(query);
    pin_ctr << "in";
    pin_ctr.close();

    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->lock_num);
    pin_ctr.open(query);
    pin_ctr << "in";
    pin_ctr.close();

    // assign pin
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->trigger_num);
    this->trigger.open(query);
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->door_num);
    this->door.open(query);
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->lock_num);
    this->lock.open(query);

    log.print_log("DONE");

    // default state : lock
    this->trigger << "1";
    this->trigger.seekg(0);


}

// doorLock class destroyer, release all pin FDs...
doorLock::~doorLock(){
    log.print_log("release resources ... ");
    this->door.close();
    this->lock.close();
    this->trigger.close();
    log.print_log("DONE");
}

// reconfigure all pin mappings, for debug only, deprecated
void doorLock::reset_pins(){
    try{
        this->door.close();
        this->lock.close();
        this->trigger.close();
    } catch(int err) {
        log.print_log("some pin's are already exist");
    };

    // create pin
    std::fstream pin_ctr;
    pin_ctr.open("/sys/class/gpio/export", std::ios::out);
    if (!pin_ctr.is_open()){
        log.print_log("\ncan not open file \"/sys/class/gpio/export\" please check again");
        std::exit(1);
    }

    pin_ctr << this->trigger_num;
    pin_ctr.seekg(0);
    pin_ctr << this->door_num;
    pin_ctr.seekg(0);
    pin_ctr << this->lock_num;
    pin_ctr.seekg(0);
    pin_ctr.close();

    // set direction
    char query[50];
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->trigger_num);
    pin_ctr.open(query);
    pin_ctr << "out";
    pin_ctr.close();

    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->door_num);
    pin_ctr.open(query);
    pin_ctr << "in";
    pin_ctr.close();

    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/direction", this->lock_num);
    pin_ctr.open(query);
    pin_ctr << "in";
    pin_ctr.close();

    // assign pin
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->trigger_num);
    this->trigger.open(query);
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->door_num);
    this->door.open(query);
    query[0] = 0;
    sprintf(query, "/sys/class/gpio/gpio%d/value", this->lock_num);
    this->lock.open(query);

    // default state : lock
    this->trigger << "1";
    this->trigger.seekg(0);
}

// refresh all pin states and save it
void doorLock::get_states(){
    this->door >> this->door_value;
    this->lock >> this->lock_value;
    this->trigger >> this->trigger_value;

    this->door.seekg(0);
    this->lock.seekg(0);
    this->trigger.seekg(0);

    this->status = lock_status(4*this->lock_value + 2*this->door_value + this->trigger_value);

    return ;
}

// unlock doorLock trigger
bool doorLock::door_open(){
    try{
        log.print_log("UNLOCK");
        this->trigger << "0";
        this->trigger.seekg(0);
    } catch (int e) {
        log.print_log("UNLOCK FAILURE");
        return false;
    }

    return true;
}

// lock doorLock trigger
bool doorLock::door_close(){
    try{
        log.print_log("LOCK");
        this->trigger << "1";
        this->trigger.seekg(0);
    } catch (int e) {
        log.print_log("LOCK FAILURE");
        return false;
    }

    return true;
}

// wait until doorLock state is in UNLOCK and OPEN
bool doorLock::wait_open(){
    get_states();
    while (this->status != UNLOCK_OPEN){
        get_states();
        usleep(1000);
    }
    log.print_log("OPEN");
    return true;
}

// wait until doorLock state is in UNLOCK and CLOSE
bool doorLock::wait_close() {
    get_states();
    while (this->status != UNLOCK_CLOSE){
        get_states();
        usleep(1000);
    }
    log.print_log("CLOSE");
    return true;
}

// wait until doorLock state is in READY STATE (LOCK CLOSE)
bool doorLock::is_ready() {
    get_states();
    if (this->status == WAIT)
        return true;
    else{
        log.print_log("door already open");
        return false;
    }

}