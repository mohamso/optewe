/**
 * @mainpage
 *
 * The hdeem_cxx header abstracts from the C hdeem API and works around quirks.
 *
 * See the example {@link hdeem_cxx.cpp} on how to use the class.
 *
 * @example hdeem_cxx.cpp
 */
#ifndef HDEEM_HDEEM_CXX_HPP
#define HDEEM_HDEEM_CXX_HPP

#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include <algorithm>

extern "C" {
#include <hdeem.h>
}
namespace hdeem
{
    namespace chrono
    {
        using clock = std::chrono::system_clock;
        using duration = std::chrono::nanoseconds;
        using time_point = std::chrono::time_point<clock, duration>;


        inline time_point convert_hdeem_time(timespec ts)
        {
            std::chrono::seconds time_seconds(ts.tv_sec);
            std::chrono::nanoseconds time_nanoseconds(ts.tv_nsec);
            auto time_since_epoch(time_seconds + time_nanoseconds);
            return time_point(time_since_epoch);
        }
    }
/**
 * General exception for all errors returned by hdeem functions.
 * For other consistency errors, std::runtime_error is used,
 * so you should catch that if you want to be sure.
 */
class error : public std::runtime_error
{
public:
    error(const std::string& what_arg, int rc)
    : std::runtime_error(what_arg + std::string(" failed with code: ") + std::to_string(rc))
    {
    }
    // TODO Add mapping for return codes
};

class overflow_error : public std::runtime_error
{
public:
    overflow_error() : std::runtime_error("Overflow during hdeem measurements.")
    {
    }
};

/**
 * Represents an identifier for a specific sensor.
 * Currently supports blade and vr sensors each with a separate index space
 */
class sensor_id
{
private:
    sensor_id(bool is_blade, int index) : is_blade_(is_blade), index_(index)
    {
    }

public:
    // Do we really have to do this just because we declare a private non-default construtor...?
    sensor_id(const sensor_id&) = default;
    sensor_id(sensor_id&&) = default;
    sensor_id& operator=(const sensor_id&) = default;
    sensor_id& operator=(sensor_id&&) = default;
    ~sensor_id() = default;

    /**
     * @return an identifier for a blade sensor with a given index
     */
    static sensor_id blade(int index = 0)
    {
        return sensor_id(true, index);
    }

    /**
     * @return an identifier for a VR sensor with a given index
     */
    static sensor_id vr(int index)
    {
        return sensor_id(false, index);
    }

    bool is_blade() const
    {
        return is_blade_;
    }

    bool is_vr() const
    {
        return !is_blade_;
    }

    int index() const
    {
        return index_;
    }

private:
    bool is_blade_;
    int index_;
};

inline std::ostream& operator<<(std::ostream& s, sensor_id sensor)
{
    s << "[" << (sensor.is_blade() ? "blade" : "vr") << ":" << sensor.index() << "]";
    return s;
}

namespace detail
{
    class single_sensor_data;
    class single_sensor_iterator : public std::iterator<std::input_iterator_tag, std::pair<size_t, float>>
    {
    public:
        single_sensor_iterator(const single_sensor_data& data, size_t index = 0) : data_(data), index_(index)
        {
        }

        single_sensor_iterator& operator++()
        {
            index_++;
            return *this;
        }

        single_sensor_iterator operator++(int)
        {
            single_sensor_iterator tmp(*this);
            operator++();
            return tmp;
        }

        bool operator==(const single_sensor_iterator& rhs)
        {
            return (index_ == rhs.index_) && (&data_ == &rhs.data_);
        }

        bool operator!=(const single_sensor_iterator& rhs)
        {
            return !(*this == rhs);
        }

        std::pair<size_t, float> operator*();

    private:
        const single_sensor_data& data_;
        size_t index_;
    };

    /**
     * A class that represents the data stream of a single sensor.
     * This object can be iterated over.
     */
    class single_sensor_data
    {
    private:
        friend class single_sensor_iterator;
        const sensor_id sensor_;
        const hdeem_global_reading_t& readings_;

    public:
        /**
         * Create a proxy for a specific sensor using the provided raw hdeem readings.
         * Note: The object keeps a reference to the readings, so it must be ensured that these
         * readings are valida s long as this object is used.
         * Note: Usually this object is usually constructed by {@link connection::get_global()}.
         * @param is_blade specify whether blade or vr sensor is used
         * @param sensor_index numerical index to the sensor within the hdeem
         */
        single_sensor_data(sensor_id sensor, const hdeem_global_reading_t& readings)
        : sensor_(sensor), readings_(readings)
        {
        }

        /**
         * @return the begin const iterator over the sensor data
         */
        single_sensor_iterator begin() const
        {
            return single_sensor_iterator(*this);
        }

        /**
         * @return the end const iterator over the sensor data
         */
        single_sensor_iterator end() const
        {
            return single_sensor_iterator(*this, size());
        }

        /**
         * @return the number of measurement values available within the readings for this sensor
         */
        size_t size() const
        {
            return sensor_.is_blade() ? readings_.nb_blade_values : readings_.nb_vr_values;
        }
    };

    inline std::pair<size_t, float> single_sensor_iterator::operator*()
    {
        float value;
        if (this->data_.sensor_.is_blade())
        {
            value = data_.readings_.blade_power[index_].value[data_.sensor_.index()];
        }
        else
        {
            assert(this->data_.sensor_.is_vr());
            value = data_.readings_.vr_power[index_].value[data_.sensor_.index()];
        }
        return std::make_pair(this->index_, value);
    }
}

/**
 * base class to Encapsulates hdeem_get_stats and hdeem_get_stats_total
 * to get measured sensor data.
 *
 * Provides sensor data per sensor(bmc/vr, index)
 */
class sensor_stats_base
{
public:
	/**
	 * Empty constructor. NEVER try to get data from objects build with this constructor.
	 * Always use connection->get_stats() or connection->get_stats_total() to get data.
	 */
	sensor_stats_base():valid_stats_(false)
	{
	}

	/**
	 * Constructor.
	 *
	 * @param bmc hdeem handler
	 */
    sensor_stats_base(hdeem_bmc_data_t& bmc) : nb_blade_sensors_(bmc.nb_blade_sensors), nb_vr_sensors_(bmc.nb_vr_sensors)
    {
    }


	/**
	 * Constructor. Build sensor_stats from another sensor_stats, and invalidate the old one.
	 *
	 * @param other sensor_stats to copy and invalidate
	 */
    sensor_stats_base(sensor_stats_base&& other)
    {
        stats_ = other.stats_;
        nb_blade_sensors_ = other.nb_blade_sensors_;
        nb_vr_sensors_ = other.nb_vr_sensors_;
        other.valid_stats_ = false;
    }

	/**
	 * Destructor. Frees data from hdeem (using hdeem_stats_free())
	 *
	 * @param bmc hdeem handler
	 */
    ~sensor_stats_base()
    {
        // We don't need to free if our object was moved from
        if (valid_stats_)
        {
            hdeem_stats_free(&stats_);
            valid_stats_ = false;
        }
    }

	/**
	 * R-Value Copy. Get data from old Object, and ivaliadte it.
	 *
	 * @param other old object
	 */
    sensor_stats_base& operator=(sensor_stats_base&& other)
    {
        assert(other.valid_stats_);
    	if (valid_stats_)
    	{
            hdeem_stats_free(&stats_);
    	}
        stats_ = other.stats_;
        nb_blade_sensors_ = other.nb_blade_sensors_;
        nb_vr_sensors_ = other.nb_vr_sensors_;
        other.valid_stats_ = false;
        valid_stats_ = true;
        return *this;
    }

    sensor_stats_base(const sensor_stats_base&) = delete;
    sensor_stats_base& operator=(const sensor_stats_base&) = delete;

    /**
     * Maximum of vr or blade power sensor values
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Maximum of vr or blade power sensor values(W)
     *
     */
    float max(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.max_blade_values.value[sensor.index()];
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.max_vr_values.value[sensor.index()];
        }
    }

    /**
     * Minimum of vr or blade power sensor values
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Minimum of vr or blade power sensor values(W)
     *
     */
    float min(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.min_blade_values.value[sensor.index()];
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.min_vr_values.value[sensor.index()];
        }
    }

    /**
     * Average of vr or blade power sensor values
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Average of vr or blade power sensor values(W)
     *
     */
    float average(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.average_blade_values.value[sensor.index()];
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.average_vr_values.value[sensor.index()];
        }
    }

    /**
     * Instant value of vr or blade power sensor
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Instant value of vr or blade power sensor values(W)
     *
     */
    float instant(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.instant_blade_values.value[sensor.index()];
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.instant_vr_values.value[sensor.index()];
        }
    }

    /**
     * Energy consumption relative to the vr or blade sensors
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Energy consumption relative to the vr or blade sensors(J)
     *
     */
    double energy(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.energy_blade_values.value[sensor.index()];
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.energy_vr_values.value[sensor.index()];
        }
    }

    /**
     * Number of samples used relative to vr or blade energy
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Number of samples used relative to vr or blade energy
     *
     */
    unsigned long long int samples_count(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return stats_.nb_blade_values;
        }
        else
        {
            assert(sensor.is_vr());
            return stats_.nb_vr_values;
        }
    }

    /*
     * Measurment timepoint when the stats are taken.
     *
     * @param sensor Sensor obejct, to the requested Sensor.
     *
     * @return Measurment timepoint when the stats are taken.
     *
     *
     */
    chrono::time_point time(sensor_id sensor) const
    {
        assert(valid(sensor));
        if (sensor.is_blade())
        {
            return chrono::convert_hdeem_time(stats_.read_time_blade);
        }
        else
        {
            return chrono::convert_hdeem_time(stats_.read_time_vr);
        }

    }

private:
    bool valid(sensor_id sensor) const
    {
        assert(valid_stats_);
        return sensor.index() < (sensor.is_blade() ? nb_blade_sensors_ : nb_vr_sensors_);
    }

    int nb_blade_sensors_;
    int nb_vr_sensors_;
    /**
     * Indicate whether the stats we have are valid. They may be invalidated if moved from or destroyed.
     */
    bool valid_stats_ = true;

protected:
    hdeem_stats_reading_t stats_;
};

/**
 * This class gets the values since the call to hdeem_start()
 * If hdeem_stop() is called it reports values up to the call of
 * hdeem_stop().
 */
class sensor_stats : public sensor_stats_base
{
public:

    sensor_stats() = default;

    /**
     * Constructor.
     *
     * Retrives the values unsing hdeem_get_stats()
     */
    sensor_stats(hdeem_bmc_data_t& bmc) : sensor_stats_base(bmc)
    {
        auto rc = hdeem_get_stats(&bmc, &stats_);
        if (rc)
        {
            throw error("hdeem_get_stats", rc);
        }
    }
};

/**
 * This class get the total energy and power (min/max/average) values.
 * These values are reported since system start.
 * Therefore, no call to hdeem_start() or hdeem_stop() is needed.
 */
class sensor_stats_total : public sensor_stats_base
{
public:

    sensor_stats_total() = default;

    /**
     * Constructor.
     *
     * Retrives the values unsing hdeem_get_stats_total()
     */
    sensor_stats_total(hdeem_bmc_data_t& bmc) : sensor_stats_base(bmc)
    {
        auto rc = hdeem_get_stats_total(&bmc, &stats_);
        if (rc)
        {
            throw error("hdeem_get_stats_total", rc);
        }
    }
};


/**
 * Encapsulates hdeem_global_reading_t struct that contains measured sensor data (for multiple sensors).
 * Provides ranges per sensor(bmc/vr, index)
 *
 * IMPORTANT: Keep this object alive as long as you are using single_sensor_data returned objects
 */
class sensor_data
{
public:
    sensor_data(hdeem_bmc_data_t& bmc) : nb_blade_sensors_(bmc.nb_blade_sensors), nb_vr_sensors_(bmc.nb_vr_sensors)
    {
        auto rc = hdeem_get_global(&bmc, &readings_);
        if (rc)
        {
            // As documented by Marc Simon in AI82:
            // > You should call hdeem_data_free(), even in the case of an error.
            // > If the error occured before the memory is allocated, the corresponding pointers are at least
            // > initialized to NULL and hdeem_data_free() will not get into segmentation violation.
            hdeem_data_free(&readings_);
            throw error("hdeem_get_global", rc);
        }
    }

    ~sensor_data()
    {
        if (valid_readings_)
        {
            hdeem_data_free(&readings_);
        }
    }
    // We need the move operators for returning from the connection object
    sensor_data(sensor_data&& other)
    {
        readings_ = other.readings_;
        nb_blade_sensors_ = other.nb_blade_sensors_;
        nb_vr_sensors_ = other.nb_vr_sensors_;
        other.valid_readings_ = false;
    }
    sensor_data& operator=(sensor_data&& other)
    {
        assert(valid_readings_);
        assert(other.valid_readings_);
        hdeem_data_free(&readings_);
        readings_ = other.readings_;
        nb_blade_sensors_ = other.nb_blade_sensors_;
        nb_vr_sensors_ = other.nb_vr_sensors_;
        other.valid_readings_ = false;
        return *this;
    }
    sensor_data& operator=(const sensor_data&) = delete;
    sensor_data& operator=(sensor_data&) = delete;

    /**
     * Return a range for the data of the selected sensor
     * @return an iterable object that provides a std::pair<size_t, float> index-value range
     */
    detail::single_sensor_data get_single_sensor_data(sensor_id sensor) const
    {
        assert(valid_readings_);
        if ((sensor.is_blade() && sensor.index() >= nb_blade_sensors_) ||
            (sensor.is_vr() && sensor.index() >= nb_vr_sensors_))
        {
            throw std::range_error("Requested invalid sensor id.");
        }

        // We give out the readings here.
        // if someone destroys our connection this thing will go amok and we won't notice.
        return detail::single_sensor_data(sensor, readings_);
    }

    size_t size(sensor_id sensor) const
    {
        return sensor.is_blade() ? readings_.nb_blade_values : readings_.nb_vr_values;
    }

private:
    hdeem_global_reading_t readings_;
    int nb_blade_sensors_;
    int nb_vr_sensors_;
    bool valid_readings_ = true;
};

/**
 * This is the main abstraction of the hdeem C interface.
 *
 * An object represents a connection to the hdeem interface. It does:
 * - provide OO access
 * - cover a couple of quirks of the raw C interface
 * - translates errors to exceptions
 * - provides sensor_data encapsulation of global readings
 *
 * Some aspects are still the lower level c-structs, e.g. {@link bmc()}, {@link get_status()}
 */
class connection
{
public:
    /**
     * Establish an in-band hdeem connection
     */
    connection()
    {
        // Probably redundant
        bmc_.host = nullptr;
        bmc_.user = nullptr;
        bmc_.password = nullptr;
        bmc_.hasGPIO = 1;
        bmc_.hasPCIe = 1;

        init();
    }

    /**
     * Establish an out-of-band hdeem connection
     * @param hostname the hostname of the BMC
     * @param username username for the BMC access
     * @param password password for the BMC access
     */
    connection(std::string hostname, std::string username, std::string password)
    {
        bmc_.host = strdup(hostname.c_str());
        bmc_.user = strdup(username.c_str());
        bmc_.password = strdup(password.c_str());
        // This has no meaning. According to the HDEEM OperatingManual V213, only the host user and password field
        // 'the user must inform'. A host != NULL and != "" implicates out of band, otherwise inband
        bmc_.hasGPIO = 0;
        bmc_.hasPCIe = 0;

        init();
    }

    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;
    // We could implement the operators here if we wanted to, but usually we don't need to.
    connection(connection&&) = delete;
    connection& operator==(connection&&) = delete;

private:
    void init()
    {
        auto ret = hdeem_init(&bmc_);
        if (ret)
        {
            throw error("Initializing hdeem (hdeem_init)", ret);
        }
    }

public:
    /**
     * Upon destruction the hdeem connection is closed
     */
    ~connection()
    {
        hdeem_close(&bmc_);
        if (bmc_.host != nullptr) {
            free(bmc_.host);
            free(bmc_.user);
            free(bmc_.password);
        }
    }

    /**
     * Start an hdeem measurement.
     * This function will retry once. If the measurement is already started, it will be restarted.
     */
    void start()
    {

        auto rc = hdeem_start(&bmc_);
        if (rc)
        {
            rc = hdeem_clear(&bmc_);
            if (rc)
            {
                throw error("hdeem_clear", rc);
            }
            rc = hdeem_start(&bmc_);
            if (rc)
            {
                throw error("hdeem_start (after clear)", rc);
            }
        }
    }

    /**
     * Stop an hdeem measurement.
     * Additional, the function waits until the polling of FPGA and BMC are complited.
     * It further checks for possible overflows. If 10 seconds after the stop, the measurement is
     * still running this function gives throws an error.
     * This method does update the internal status, so you can call {@link get_status(false)} afterwards.
     */
    void stop()
    {
        auto rc = hdeem_stop(&bmc_);
        if (rc)
        {
            throw error("hdeem_stop", rc);
        }
        const int max_retries = 100;
        const auto sleep_time = std::chrono::milliseconds(100);
        for (int retry = 0; retry < max_retries; retry++)
        {
            rc = hdeem_check_status(&bmc_, &status_);
            if (rc)
            {
                throw error("hdeem_check_status", rc);
            }
            auto& status = status_.status;

            if (!IsBmcPolling(status) && !IsFpgaPolling(status))
            {
                // The stop was actually successful
                // Can we handle overflows here better?
                if (IsBmcOverflow(status) || IsFpgaBladeOverflow(status) || IsFpgaVrOverflow(status))
                {
                    throw overflow_error();
                }
                return;
            }

            std::this_thread::sleep_for(sleep_time);
        }
        throw std::runtime_error("Measurement is still polling after stop.");
    }

    /**
     * Fetch the index/value measurement log.
     * @return a {@link sensor_data} object encapsulating the timelines of different sensors
     */
    sensor_data get_global()
    {
        return sensor_data(bmc_);
    }

    /**
     * Update and return the internal status.
     * This is a copy of an an ugly C struct.
     *
     * @param refresh if set to false, the most recent status is returned.
     */
    hdeem_status_t get_status(bool refresh = true)
    {
        if (refresh)
        {
            auto rc = hdeem_check_status(&bmc_, &status_);
            if (rc)
            {
                throw error("hdeem_check_status", rc);
            }
        }
        return status_;
    }

    /**
     * Reports the energy and power (min/max/average/instant) values from hdeem
     *
     * The values are reportet since the call to conection::start().
     * When connection::stop() is called, the values don't change
     * anymore.
     *
     * @return energy and power (min/max/average) values from hdeem since
     *         conection::start(). Maximal till the call to
     *         connection::stop()
     */
    sensor_stats get_stats()
    {
        return sensor_stats(bmc_);
    }

    /**
     * Reports the energy and power (min/max/average/instant) values from hdeem
     *
     *
     * The values are reported since the start of the sytem. Therefore,
     * a previous call to connection::start() and conection::stop() is not
     * necesarry.
     *
     * @return energy and power (min/max/average) values from hdeem since
     *         system start.
     *
     */
    sensor_stats_total get_stats_total()
    {
        return sensor_stats_total(bmc_);
    }


    /**
     * @param sensor the id of the sensor
     * @return the name of the sensor which is upper case.
     */
    std::string sensor_name(sensor_id sensor) const
    {
        std::string name_upper;
        if (sensor.is_blade()) {
            assert(sensor.index() < bmc_.nb_blade_sensors);
            name_upper = std::string(bmc_.name_blade_sensors[sensor.index()]);
        } else {
            assert(sensor.is_vr());
            assert(sensor.index() < bmc_.nb_vr_sensors);
            name_upper = std::string(bmc_.name_vr_sensors[sensor.index()]);
        }
        std::transform(name_upper.begin(), name_upper.end(), name_upper.begin(),
                       ::toupper);
        return name_upper;
    }

    /**
     * @param sensor the id of the sensor
     * @return the real name of the sensor as given by hdeem
     */
    std::string sensor_real_name(sensor_id sensor) const
    {
        if (sensor.is_blade()) {
            assert(sensor.index() < bmc_.nb_blade_sensors);
            return bmc_.name_blade_sensors[sensor.index()];
        } else {
            assert(sensor.is_vr());
            assert(sensor.index() < bmc_.nb_vr_sensors);
            return bmc_.name_vr_sensors[sensor.index()];
        }
    }

	/**
	 * @param sensor_id id of the sensor.
	 * @retrun sampling rate in samples per second
	 */
    double sensor_sampling_rate(sensor_id sensor) const
    {
    	if (sensor.is_blade()) {
    		return 1000;
    	} else {
            assert(sensor.is_vr());
            return 100;
        }
    }

    /**
     * @return a collection of {@link sensor_id}s. Not guaranteed to always be a vector.
     */
    std::vector<sensor_id> sensors() const
    {
        std::vector<sensor_id> ret;
        for (int idx = 0; idx < bmc_.nb_blade_sensors; idx++) {
            ret.emplace_back(sensor_id::blade(idx));
        }
        for (int idx = 0; idx < bmc_.nb_vr_sensors; idx++) {
            ret.emplace_back(sensor_id::vr(idx));
        }
        return ret;
    }

    /**
     * Return the internal bmc object.
     * Do not use this function unless you really know what you are doing.
     * If you read this, you probably should not use it.
     */
    hdeem_bmc_data_t& bmc()
    {
        return bmc_;
    }

private:
    bool valid_readings_ = false;

    hdeem_bmc_data_t bmc_;
    hdeem_status_t status_;

    friend std::ostream& operator<<(std::ostream& s, const connection& hdeem);
};

inline std::ostream& operator<<(std::ostream& s, const connection& hdeem)
{
    s << "Hdeem connection ";
    if (hdeem.bmc_.host)
    {
        s << "oob to " << hdeem.bmc_.host << ".";
    }
    else
    {
        s << "inband.";
    }
    return s;
}
} // namespaec hdeem

#endif // HDEEM_HDEEM_CXX_HPP
