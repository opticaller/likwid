/*
 * =======================================================================================
 *
 *      Filename:  access_x86.c
 *
 *      Description:  Interface to x86 related functions for the access module.
 *
 *      Version:   <VERSION>
 *      Released:  <DATE>
 *
 *      Author:   Thomas Roehl (tr), thomas.roehl@googlemail.com
 *      Project:  likwid
 *
 *      Copyright (C) 2015 RRZE, University Erlangen-Nuremberg
 *
 *      This program is free software: you can redistribute it and/or modify it under
 *      the terms of the GNU General Public License as published by the Free Software
 *      Foundation, either version 3 of the License, or (at your option) any later
 *      version.
 *
 *      This program is distributed in the hope that it will be useful, but WITHOUT ANY
 *      WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *      PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along with
 *      this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =======================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#include <types.h>
#include <error.h>
#include <topology.h>
#include <access.h>
#include <access_x86.h>
#include <access_x86_msr.h>
#include <access_x86_pci.h>
#include <affinity.h>



int access_x86_init(int cpu_id)
{
    int ret = access_x86_msr_init(cpu_id);
    if (ret == 0)
    {
        if (cpuid_info.supportUncore)
        {
            ret = access_x86_pci_init(affinity_core2node_lookup[cpu_id]);
        }
    }
    return ret;
}

int access_x86_read(PciDeviceIndex dev, const int cpu_id, uint32_t reg, uint64_t *data)
{
    int err;
    uint64_t tmp = 0x0ULL;
    if (dev == MSR_DEV)
    {
        err = access_x86_msr_read(cpu_id, reg, &tmp);
        *data = tmp;
    }
    else
    {
        if (access_x86_pci_check(dev, affinity_core2node_lookup[cpu_id]))
        {
            err = access_x86_pci_read(dev, affinity_core2node_lookup[cpu_id], reg, &tmp);
            *data = tmp;
        }
    }
    return err;
}

int access_x86_write(PciDeviceIndex dev, const int cpu_id, uint32_t reg, uint64_t data)
{
    int err;
    if (dev == MSR_DEV)
    {
        err = access_x86_msr_write(cpu_id, reg, data);
    }
    else
    {
        if (access_x86_pci_check(dev, affinity_core2node_lookup[cpu_id]))
        {
            err = access_x86_pci_write(dev, affinity_core2node_lookup[cpu_id], reg, data);
        }
    }
    return err;
}

void access_x86_finalize(int cpu_id)
{
    access_x86_msr_finalize(cpu_id);
    if (cpuid_info.supportUncore)
    {
        access_x86_pci_finalize(affinity_core2node_lookup[cpu_id]);
    }
}

int access_x86_check(PciDeviceIndex dev, int cpu_id)
{
    if (dev == MSR_DEV)
    {
        return access_x86_msr_check(dev, cpu_id);
    }
    else
    {
        return access_x86_pci_check(dev, affinity_core2node_lookup[cpu_id]);
    }
    return 0;
}
