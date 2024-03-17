// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.25;

abstract contract Whitelist {
    address[] internal _whitelist;

    function _add(address _wl) internal {
        for(uint i = 0; i < _whitelist.length; i++)
            if (_whitelist[i] == _wl) return;
        _whitelist.push(_wl);
    }

    function AddToWhitelist(address _wl) public {
        bool May = false;
        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) return;
            if(a == msg.sender) May = true;
        }
        assert(May == true);
        _whitelist.push(_wl);
    }

    function AssertWhitelisted(address _wl) public view {
        bool May = false;
        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) May = true;
        }
        assert(May == true);
    }

    function _remove(uint v) private {
        _whitelist[v] = _whitelist[_whitelist.length - 1];
        _whitelist.pop();
    }

    function RemoveFromWhitelist(address _wl) public {
        bool May = false;
        uint v = 99999;

        for(uint i = 0; i < _whitelist.length; i++) {
            address a = _whitelist[i];
            if (a == _wl) v = i;
            if(a == msg.sender) May = true;
        }
        assert(May == true);
        _remove(v);
    }

    function GetWhitelistByIndex(uint256 i) public view returns(address) {
        return _whitelist[i];
    }

    function WhitelistCount() public view returns(uint256) {
        return _whitelist.length;
    }
}
