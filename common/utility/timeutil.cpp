//Copyright © 2023 Charles Kerr. All rights reserved.

#include "timeutil.hpp"

#include <algorithm>
#include <stdexcept>

#include "strutil.hpp"

using namespace std::string_literals ;

//======================================================================
namespace util {
    //======================================================================
    //======================================================================
    HourMinute::HourMinute():hour(0),minute(0){
    }
    
    //======================================================================
    HourMinute::HourMinute(int hour, int minute):hour(hour),minute(minute){
    }
    
    //======================================================================
    HourMinute::HourMinute(const ourclock::time_point &timePoint):HourMinute() {
        this->load(timePoint) ;
    }
    
    //======================================================================
    HourMinute::HourMinute(const std::string &timeString):HourMinute() {
        this->load(timeString) ;
    }
    
    //======================================================================
    auto HourMinute::describe() const -> std::string {
        return util::format("%0.2i:%0.2i", hour,minute) ;
    }
    
    //======================================================================
    auto HourMinute::load(const std::string &timeString) ->void {
        auto [shour,sminute] = util::split(timeString,":") ;
        try {
            this->hour = std::stoi(shour,nullptr,10) ;
            this->minute = std::stoi(sminute,nullptr,10) ;
        }
        catch(...){
            throw std::runtime_error("Error converting time string: "s + timeString);
        }
        if (this->hour < 0 || this->hour > 23 || this->minute < 0 || this->minute > 59) {
            throw std::runtime_error("Error converting time string: "s + timeString);
        }
    }
    
    //======================================================================
    auto HourMinute::load(const ourclock::time_point &timePoint) -> void {
        std::stringstream output ;
        tm myvalue;
        auto format = "%H:%M"s;
        auto time = ourclock::to_time_t(timePoint);
#if defined(_MSC_VER)
        auto status = ::localtime_s(&myvalue, &time);
#else
        ::localtime_r(&time, &myvalue);
#endif
        output << std::put_time(&myvalue, format.c_str());
        load(output.str()) ;
    }
    
    //====================================================================================
    auto HourMinute::operator==(const HourMinute &value) const ->bool {
        return this->hour == value.hour && this->minute == value.minute ;
    }
    
    //====================================================================================
    auto HourMinute::operator<(const HourMinute &value) const ->bool {
        if ( this->hour < value.hour) {
            return true ;
        }
        else  if (this->hour == value.hour){
            return this->minute < value.minute;
        }
        return false ;
    }
    
    //====================================================================================
    auto HourMinute::operator>(const HourMinute &value) const -> bool {
        return !operator<=(value) ;
    }
    
    //====================================================================================
    auto HourMinute::operator<=(const HourMinute &value) const ->bool {
        return operator==(value) || operator<(value) ;
    }
    
    //====================================================================================
    auto HourMinute::operator>=(const HourMinute &value) const ->bool {
        return operator==(value) || operator>(value) ;
    }
    
    // So we can add/subtract minutes from the time
    //====================================================================================
    auto HourMinute::operator+(int minutes) const -> HourMinute {
        auto newMinute = this->minute + minutes ;
        auto hoursToAdd = newMinute / 60 ; // 60 minutes to an hour
        newMinute = newMinute % 60 ;
        auto newHour = this->hour + hoursToAdd ;
        // We need to check for overflow
        if (newHour >= 24) {
            newHour -= 24 ;
        }
        return HourMinute(newHour,newMinute) ;
    }
    
    //====================================================================================
    auto HourMinute::operator-(int minutes) const -> HourMinute {
        auto newHour = this->hour ;
        auto newMinute = this->minute ;
        // Are we going to need to borrow an hour?
        if (newMinute < minutes) {
            newHour -= 1;
        }
        newMinute -= minutes ;
        
        if (newHour < 0) {
            // We need to do something
            newHour = 23 ;
        }
        if (newMinute < 0)  {
            newMinute = 60 + newMinute ;
        }
        return HourMinute(newHour,newMinute);
    }
    
    //====================================================================================
    auto HourMinute::operator-=(int minutes) ->HourMinute& {
        auto temp = this->operator-(minutes) ;
        this->hour = temp.hour ;
        this->minute = temp.minute ;
        return *this ;
    }
    
    //====================================================================================
    auto HourMinute::operator+=(int minutes) ->HourMinute& {
        auto temp = this->operator+(minutes) ;
        this->hour = temp.hour ;
        this->minute = temp.minute ;
        return *this ;
    }
    
    //====================================================================================
    auto HourMinute::operator=(const std::string& value) -> HourMinute& {
        this->load(value) ;
        return *this ;
    }
    //====================================================================================
    auto HourMinute::operator-(const HourMinute &value) const -> int {
        auto hours = this->hour - value.hour ;
        
        auto minutes = this->minute - value.minute ;
        if (minutes < 0) {
            hours -= 1 ;
            minutes += 60 ;
        }
        return (hours * 60 ) + minutes ;
    }
    
    //====================================================================================
    auto HourMinute::now() -> HourMinute {
        return HourMinute(ourclock::now()) ;
    }
    
    // ===========================================================================================
    // HourRange
    // ===========================================================================================
    // ===========================================================================================
    HourRange::HourRange(const HourMinute &start, const HourMinute &end):startTime(start),endTime(end){
        
    }
    
    // ===========================================================================================
    HourRange::HourRange(const std::string & line ) {
        auto [beg,end] = split(line,",") ;
        startTime = HourMinute(beg) ;
        endTime = HourMinute(end) ;
    }
    
    // ===========================================================================================
    auto HourRange::inRange(const ourclock::time_point &now ) const -> bool {
        auto hnow = HourMinute::now() ;
        auto value = startTime <= hnow ;
        value = value &&  (endTime > hnow );
        return value ;
    }
    // ===========================================================================================
    auto HourRange::inRange(const HourMinute &value) const -> bool {
        auto rvalue = startTime <= value ;
        rvalue = rvalue && ( this->endTime > value) ;
        return rvalue ;
    }
    // ===========================================================================================
    // MonthDay
    // ===========================================================================================
    
    // ===========================================================================================
    auto MonthDay::convert(const std::string &line) -> void {
        try {
            auto [smonth,sday] = util::split(line,":") ;
            month = std::stoi(smonth,nullptr,10) ;
            day = std::stoi(sday,nullptr,10) ;
        }
        catch(...){
            throw std::runtime_error("Error converting month/day") ;
        }
    }
    
    // ===========================================================================================
    MonthDay::MonthDay(const ourclock::time_point &now){
        
        this->convert(sysTimeToString(now,"%m:%d")) ;
    }
    
    // ===========================================================================================
    MonthDay::MonthDay(int monthvalue, int dayvalue):month(monthvalue),day(dayvalue) {
        
    }
    
    // ===========================================================================================
    MonthDay::MonthDay(const std::string &line):MonthDay() {
        this->convert(line) ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator<(const MonthDay &value) const -> bool {
        if (this->month > value.month){
            return false ;
        }
        if (this->month == value.month){
            if (this->day >= value.day){
                return false ;
            }
        }
        return true ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator==(const MonthDay &value) const -> bool {
        return (this->month == value.month) && (value.day == this->day) ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator!=(const MonthDay &value) const -> bool {
        return !operator==(value) ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator<=(const MonthDay &value) const -> bool {
        return operator<(value) || operator==(value) ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator>(const MonthDay &value) const -> bool {
        return !operator<(value) && !operator==(value) ;
    }
    
    // ===========================================================================================
    auto MonthDay::operator>=(const MonthDay &value) const -> bool {
        return !operator<(value)  ;
    }
    
    // ===========================================================================================
    auto MonthDay::describe() const -> std::string {
        return format("%0.2i:%0.2i", month,day) ;
    }
    
    // ===========================================================================================
    // MonthRange
    // ===========================================================================================
    
    // ===========================================================================================
    MonthRange::MonthRange() {
        
    }
    // ===========================================================================================
    MonthRange::MonthRange(const MonthDay &beg, const MonthDay &end):startDay(beg),endDay(end){
        
    }
    // ===========================================================================================
    MonthRange::MonthRange(const std::string &line) {
        auto [beg,end]  = util::split(line,",") ;
        startDay = MonthDay(beg) ;
        endDay = MonthDay(end) ;
    }
    
    // ===========================================================================================
    auto MonthRange::inRange(const ourclock::time_point &now  ) const -> bool {
        // first determine the month to use for the start
        auto hnow = MonthDay(now) ;
        if (hnow >= startDay && hnow <= endDay) {
            return true ;
        }
        else  if (startDay != endDay){
            // So we are not in range, but, what if the startday is greater then endday year wrap
            if (endDay < startDay) {
                if ( (hnow <= endDay) || (hnow >= startDay) ) {
                    return true ;
                }
            }
        }
        return false ;
    }
}
