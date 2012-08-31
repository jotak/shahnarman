childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Dealer d'EPO"
	else
		return "EPO dealer"
	end
end

function getChildEffectsName()
	if language == "french" then
		return "EPO"
	else
		return "EPO"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : Xµ. L'endurance de l'unité ciblée augmente de X ce tour-ci (X étant déterminé par le mana payé)."
	else
		return "Activation cost: Xµ. Endurance of target unit is increased by X this turn (X being paid mana)."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		str = ""
		if language == "french" then
			str = "L'endurance de la cible augmentera de 1 par point de mana."
		else
			str = "Target endurance will increase by 1 for each 1 mana spent."
		end
		askForExtraMana(str, 4, 0, -1, "onGotMana")	-- 1st arg is mana color bitfield (1 life, 2 law, 4 death, 8 chaos) ; 2nd is min, 3rd is max, 4th is callback
	end
end

function onGotMana(value)
	addResolveParameter(value)
	selectTarget("unit", "inrange")
end

nbEPO = 0
EPOLifetime = 1
currentEPO = {}

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolveChild(effect, params)
	if effect == 1 then
		power, player, unit = split(params, " ");
		addChildEffectToUnit(1, player, unit)
		-- store active steroids to remove them later
		nbEPO = nbEPO + 1
		currentEPO[nbEPO] = {player, unit, power, 0}
		dispatchToClients("onEPOAdded", player, unit, power)
	end
end

-- The two following variables are used to solve a problem: if this EPO was activated twice (or more) on the same unit,
	-- then we need to know if the call of "child_getMod_endurance" results of the first child effect or the second one.
	-- Since they can have each a different "power" (depending on the mana spent for each one), it's important to be able
	-- to differentiate them. That's why a call of "child_getMod_xxx" gives in parameter a "getModInstance". In C++, when
	-- we need to know the endurance of a unit we call a function (let's call it "parent call") which then calls 
	-- "child_getMod_endurance" for each child effect attached. So if it's called twice for the same "parent call", then
	-- the same "getModInstance" id will be provided. If it's for two different "parent calls" then two different
	-- getModInstance will be provided.
-- All this shit is only useful because we use custom mana cost and thus two child effects are not identical. In most cases, it's much simpler!
tmp_getModInstance = -1		-- last instance id
tmp_getModCount = 0			-- counter in current instance id
function child_getMod_endurance(effect, value, params, getModInstance)
	if effect == 1 then
		-- parse params (it contains target identifiers)
		objtype, player, unit = split(params, " ")
		if objtype == "unit" then
			-- check getModInstance
			if getModInstance ~= tmp_getModInstance then
				-- new instance, so reset tmp_getModCount
				tmp_getModCount = 0
				tmp_getModInstance = getModInstance
			end
			iCount = 0
			-- look in active EPOs if we can find this unit
			for i = 1, nbEPO do
				if currentEPO[i][1] == player and currentEPO[i][2] == unit then	-- forced conversion in integer is needed here
					-- this is our unit
					if tmp_getModCount == iCount then
						-- good child instance
						tmp_getModCount = tmp_getModCount + 1
						return value + currentEPO[i][3]	-- currentEPO[i][3] is EPO power
					else
						iCount = iCount + 1	-- increase counter until it reached tmp_getModCount
					end
				end
			end
		end
	end
	return value
end

function onNewUnitTurn()
	-- increase turn counter of every active steroid
	i = 1
	while i <= nbEPO do
		currentEPO[i][4] = currentEPO[i][4] + 1
		-- remove if too old
		if currentEPO[i][4] > EPOLifetime then
			removeChildEffectFromUnit(1, currentEPO[i][1], currentEPO[i][2])
			dispatchToClients("onEPORemoved", i)
			table.remove(currentEPO, i)
			nbEPO = nbEPO - 1
			i = i - 1
		end
		i = i + 1
	end
end

function onEPOAdded(player, unit, power)
	nbEPO = nbEPO + 1
	currentEPO[nbEPO] = {player, unit, power, 0}
--	print("EPOAdded ; player=", player, " ; unit=", unit, " ; power=", power)
--	print("nbEPO=", nbEPO)
--	dbg_printEPO()
	return 0	-- no need to reload LUA basic data
end

function onEPORemoved(player, unit, power)
--	print("onEPORemoved")
	table.remove(currentEPO, i)
	nbEPO = nbEPO - 1
	return 0	-- no need to reload LUA basic data
end

function dbg_printEPO()
	for i = 1, nbEPO do
		print("i=", i, " ; currentEPO[i][1]=", currentEPO[i][1], " ; currentEPO[i][2]=", currentEPO[i][2], " ; currentEPO[i][3]=", currentEPO[i][3])
	end
end