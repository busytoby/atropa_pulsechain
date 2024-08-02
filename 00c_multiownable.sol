// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import {Context} from "@openzeppelin/contracts/utils/Context.sol";

abstract contract MultiOwnable is Context {
    mapping(address => bool) private _owners;

    error OwnableUnauthorizedAccount(address account);
    error OwnableInvalidOwner(address owner);
    event OwnershipUpdate(address indexed newOwner, bool indexed state);

    constructor(address initialOwner) {
        if (initialOwner == address(0)) {
            revert OwnableInvalidOwner(address(0));
        }
        _changeOwnership(initialOwner, true);
    }

    modifier onlyOwner() {
        _checkOwner();
        _;
    }

    function owner(address cOwner) public view virtual returns (bool) {
        return _owners[cOwner];
    }

    function _checkOwner() internal view virtual {
        if (!owner(_msgSender())) {
            revert OwnableUnauthorizedAccount(_msgSender());
        }
    }

    function renounceOwnership(address toRemove) public virtual onlyOwner {
        _changeOwnership(toRemove, false);
    }

    function addOwner(address newOwner) public virtual onlyOwner {
        if (newOwner == address(0)) {
            revert OwnableInvalidOwner(address(0));
        }
        _changeOwnership(newOwner, true);
    }

    function _changeOwnership(address cOwner, bool cState) internal virtual {
        _owners[cOwner] = cState;
        emit OwnershipUpdate(cOwner, cState);
    }
}
